========================================================================
    活动模板库：DmsStartUI 项目概述
========================================================================

应用程序向导已为您创建了此 DmsStartUI 项目，您可以在此基础之上
编写服务 (EXE)。


此文件概述了组成项目的每个文件
的内容。

DmsStartUI.vcproj
    这是使用应用程序向导生成的 VC++ 项目的主项目文件。 
    它包含有关生成该文件所使用的 Visual C++ 版本的信息，以及
    有关在应用程序向导中选择的平台、配置和项目功能
    的信息。

DmsStartUI.idl
    此文件包含在您的项目中定义的类型库、接口
    和 coclass 的 IDL 定义。
    此文件将由 MIDL 编译器处理以生成：
        C++ 接口定义和 GUID 声明                 (DmsStartUI.h)
        GUID 定义                                (DmsStartUI_i.c)
        一个类型库                               (DmsStartUI.tlb)
        封送代码                                 (DmsStartUI_p.c 和 dlldata.c)

DmsStartUI.h
    此文件包含在 DmsStartUI.idl 中定义的项的 C++ 接口定义
    和 GUID 声明。它将在编译过程中由 MIDL 重新生成。

DmsStartUI.cpp
    此文件包含对象映射以及 WinMain、ServiceMain 和
    服务管理函数的实现。

DmsStartUI.rc
    这是程序使用的所有 Microsoft Windows 资源的清
    单。


/////////////////////////////////////////////////////////////////////////////
其他标准文件：

StdAfx.h，StdAfx.cpp
    这些文件用于生成名为 DmsStartUI.pch 的预编译头 (PCH) 文件
    和名为 StdAfx.obj 的预编译类型文件。

Resource.h
    这是用于定义资源 ID 的标准头文件。

/////////////////////////////////////////////////////////////////////////////
代理/存根 (stub) DLL 项目和模块定义文件：

DmsStartUIps.vcproj
    此文件是用于生成代理/存根 (stub) DLL 的项目文件（若有必要）。
	主项目中的 IDL 文件必须至少包含一个接口，并且
	在生成代理/存根 (stub) DLL 之前必须先编译 IDL 文件。	此进程生成
	dlldata.c、DmsStartUI_i.c 和 DmsStartUI_p.c，它们。
	是生成代理/存根 (stub) DLL 所必需的。

DmsStartUIps.def
    此模块定义文件为链接器提供有关代理/存根 (stub) 所要求的
    导出的信息。

/////////////////////////////////////////////////////////////////////////////
