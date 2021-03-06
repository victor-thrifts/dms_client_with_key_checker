#include "StdAfx.h"
#include "BakcupsFile.h"
#include "Support.h"
#include "Monitor.h"
#include "FileMonitor.h"
#include <shlwapi.h>



CBakcupsFile::CBakcupsFile(void):m_Crypt(12345678)
{
	//StartThread();
}

CBakcupsFile::~CBakcupsFile(void)
{
}

UINT CBakcupsFile::CopyFileThread(LPVOID lpvThis){

	CBakcupsFile* pThis = (CBakcupsFile*)lpvThis;
	while (TRUE)
	{
		Sleep(2000);
		pThis->IsCopyFile();

	}
	return 0;
}

void  CBakcupsFile::StartThread(){
	CWinThread * pThread = AfxBeginThread(CopyFileThread, this);
	if( !pThread )
	{//couldn't create the thread!
		//TRACE(_T("CDirectoryChangeWatcher::WatchDirectory()-- AfxBeginThread failed!\n"));
		//pDirInfo->DeleteSelf( NULL );
		//return (GetLastError() == ERROR_SUCCESS)? ERROR_MAX_THRDS_REACHED : GetLastError();
	}
	else
	{
		m_hThread	 = pThread->m_hThread;
		m_dwThreadID = pThread->m_nThreadID;
		//pThread->m_bAutoDelete = TRUE;//pThread is deleted when thread ends....it's TRUE by default(for CWinThread ptrs returned by AfxBeginThread(threadproc, void*)), but just makin sure.

	}
}

CString CBakcupsFile::GetFileExitName(CString name){
	int pos= name.ReverseFind(_T('\\')) + 1;
	int len = name.GetLength();
	CString fileName = name.Right(len - pos);
	return fileName;
}

void CBakcupsFile::IsCopyFile(){
	ULONGLONG time = CTime::GetCurrentTime().GetTime();

	std::map<CString,BACK_FILE_INFO> ::iterator iter;
	CString key;
	CLock lock(m_Lock);
	CString bufDir = CSupport::GetBufDir() + "\\";
	CString strFileName("");
	int nTimeLen = CSupport::GetExtendBackupTime()/1000;

	if (nTimeLen <= 0)
	{
		nTimeLen = 1;
	}
	vector<CString> vecDel;
	for(iter = m_MapFile.begin(); iter != m_MapFile.end(); iter ++){
		BACK_FILE_INFO& info = iter->second;
		key = iter->first;


		if(time - info.time > nTimeLen){
			strFileName = bufDir + info.strNewName;
			//HANDLE handle = CreateFile(strFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			//if(INVALID_HANDLE_VALUE == handle){
			//	TRACE("no copy %64d----%s\t%s\n",info.id,info.strFileName,info.strNewName);
			//	CloseHandle(handle);
			//	continue;
			//}

			//CloseHandle(handle);
			if(::CopyFile(info.strFileName,strFileName,false)){
				//info.isCopyFinsh = TRUE;
				//info.id = 0;
				vecDel.push_back(key);
				TRACE("copy %I64d----%s\t%s\n",info.id,info.strFileName,info.strNewName);
			}

			//if(info.dw == 3){
				CString fileTmp = this->GetFileTmpPath(info.strFileName);
				if(fileTmp.IsEmpty())
					continue ;
				CString fileName =  GetFileExitName(info.strFileName);
				fileTmp += fileName + ".tmp";
				CTime time(info.time);
				CFileMonitor::WriteTmpFile(info.strFileName,fileTmp,info.strNewName,info.id,time);
			//}
		}
	}

	int cnt = vecDel.size();
	for (int ii = 0 ; ii < cnt ;ii ++)
	{
		m_MapFile.erase(vecDel[ii]);
	}

}


CString CBakcupsFile::GetFileTmpPath(const CString& oldName)
{
	int index, start = 0;
	CString strTmp;
	CString strBackupsPath = CSupport::GetBufDir() + "\\";
	CString strNewPathEx = oldName;
	strNewPathEx.Replace(":","");

	while ( (index = strNewPathEx.Find('\\', start)) != -1) {	
		strTmp = strNewPathEx.Mid(start,index - start);
		strTmp = m_Crypt.Encrypt(strTmp);
		strBackupsPath += strTmp;

		if(!PathFileExists(strBackupsPath))//文件夹不存在则创建
			CreateDirectory(strBackupsPath, NULL);

		start = index + 1; 
		strBackupsPath += "\\";

	}
	
	return strBackupsPath;
}



CString CBakcupsFile::AddFile(CTime tTime,int dwType,CString strFile,ULONGLONG id)
{
	CLock lock(m_Lock);
	if(m_MapFile.find(strFile) == m_MapFile.end()){

		CString strPath = tTime.Format("%y%m%d%H");//格式化日期时间 
		CString strUUID = strPath + "\\" + GUID_Generator();
		strPath = CSupport::GetBufDir() + "\\" +strPath;
		if(!PathFileExists(strPath))//文件夹不存在则创建
			CreateDirectory(strPath, NULL);

		BACK_FILE_INFO& info = m_MapFile[strFile];
		m_MapFile[strFile].time = tTime.GetTime();
		m_MapFile[strFile].id = id;
		info.dw = dwType;
		info.strFileName = strFile;
		info.strNewName = strUUID;

		return strUUID;
	}else{
		BACK_FILE_INFO& info = m_MapFile[strFile];
		ULONGLONG ll = tTime.GetTime();

		if(ll > m_MapFile[strFile].time){
			m_MapFile[strFile].time = ll;
			m_MapFile[strFile].id = id;
		}else if(ll == m_MapFile[strFile].time){
			if(id > m_MapFile[strFile].id){
				m_MapFile[strFile].time = ll;
				m_MapFile[strFile].id = id;
			}
		}

		info.isCopyFinsh = FALSE;

		return info.strNewName;
	}

}
