; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CJllServerView
LastTemplate=CComboBox
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Jll Server.h"
LastPage=0

ClassCount=10
Class1=CJllServerApp
Class2=CJllServerDoc
Class3=CJllServerView
Class4=CMainFrame

ResourceCount=8
Resource1=IDD_FILEOPENORD
Class5=CAboutDlg
Resource2=IDR_TASKBAR_MENU
Resource3=IDD_DIALOGBAR (English (U.S.))
Resource4=IDD_ABOUTBOX
Resource5=IDD_CHOOSEDIR_DIALOG (English (U.S.))
Class6=CChooseDirDlg
Class7=CNibbleModeProto
Class8=CExceptDlg
Resource6=IDD_DIALOGBAR
Class9=CButtonStyle
Resource7=IDR_MAINFRAME
Class10=CFolderCombo
Resource8=IDD_EXCEPTBOX

[CLS:CJllServerApp]
Type=0
HeaderFile=Jll Server.h
ImplementationFile=Jll Server.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CJllServerDoc]
Type=0
HeaderFile=Jll ServerDoc.h
ImplementationFile=Jll ServerDoc.cpp
Filter=C
LastObject=CJllServerDoc
BaseClass=CDocument
VirtualFilter=DC

[CLS:CJllServerView]
Type=0
HeaderFile=Jll ServerView.h
ImplementationFile=Jll ServerView.cpp
Filter=W
BaseClass=CFormView
VirtualFilter=VWC
LastObject=CJllServerView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=W
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=ID_APP_EXIT




[CLS:CAboutDlg]
Type=0
HeaderFile=Jll Server.cpp
ImplementationFile=Jll Server.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC_VERSION,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_SAVE_AS
Command5=ID_FILE_MRU_FILE1
Command6=ID_APP_EXIT
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_EDIT_COPY
Command10=ID_EDIT_PASTE
Command11=ID_EDIT_DISABLEWARMPOLL
Command12=ID_VIEW_TOOLBAR
Command13=ID_VIEW_STATUS_BAR
Command14=ID_APP_ABOUT
CommandCount=14

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
Command9=ID_DETECT_SPK_ON
Command10=ID_DETECT_SPK_OFF
CommandCount=10

[DLG:IDD_DIALOGBAR (English (U.S.))]
Type=1
ControlCount=3
Control1=IDC_BUTTON_FOR_DIR,button,1342245888
Control2=IDC_STATIC,static,1342308876
Control3=IDC_STATIC,static,1342308876
Class=CJllServerView

[DLG:IDD_DIALOGBAR]
Type=1
Class=CJllServerView
ControlCount=6
Control1=IDC_BUTTON_FOR_DIR,button,1342245888
Control2=IDC_STATIC,static,1342308864
Control3=IDC_GRP_FRAME,button,1342308359
Control4=IDC_BUTTON_RESET_DIR,button,1342242827
Control5=IDC_BUTTON_SET_DIR,button,1342242827
Control6=IDC_COMBO_FOR_DIR,combobox,1344340227

[DLG:IDD_CHOOSEDIR_DIALOG (English (U.S.))]
Type=1
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_DIRECTORY_NAME,edit,1350633600
Control4=IDC_DIRECTORY_TREE,SysTreeView32,1342242834
Control5=IDC_DRIVE_COMBO,combobox,1344340226
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Class=CChooseDirDlg

[CLS:CChooseDirDlg]
Type=0
HeaderFile=ChooseDirDlg.h
ImplementationFile=ChooseDirDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CChooseDirDlg
VirtualFilter=dWC

[CLS:CNibbleModeProto]
Type=0
HeaderFile=parallel.h
ImplementationFile=parallel.cpp
BaseClass=CStatic
Filter=W
LastObject=CNibbleModeProto

[CLS:CExceptDlg]
Type=0
HeaderFile=except.h
ImplementationFile=_except.cpp
BaseClass=CDialog
LastObject=CExceptDlg
Filter=D
VirtualFilter=dWC

[DLG:IDD_EXCEPTBOX]
Type=1
Class=CExceptDlg
ControlCount=3
Control1=IDOK,button,1342242817
Control2=IDR_APPICON2,static,1342177283
Control3=IDC_EXCEPTEDIT,edit,1352730692

[DLG:IDD_FILEOPENORD]
Type=1
Class=CExceptDlg
ControlCount=14
Control1=1090,static,1073872896
Control2=1152,edit,1082197120
Control3=1120,listbox,1084297299
Control4=65535,static,1342308352
Control5=1088,static,1342308480
Control6=1121,listbox,1352732755
Control7=1089,static,1073872896
Control8=1136,combobox,1084293187
Control9=1091,static,1342308352
Control10=1137,combobox,1352729427
Control11=IDOK,button,1342373889
Control12=IDCANCEL,button,1342373888
Control13=1038,button,1342373888
Control14=1040,button,1342373891

[CLS:CButtonStyle]
Type=0
HeaderFile=ButtonStyle.h
ImplementationFile=ButtonStyle.cpp
BaseClass=CButton
Filter=W
VirtualFilter=BWC
LastObject=CButtonStyle

[MNU:IDR_TASKBAR_MENU]
Type=1
Class=?
Command1=ID_APP_ABOUT
Command2=IDR_TASKBAR_MENU_SHOW
Command3=ID_APP_EXIT
CommandCount=3

[CLS:CFolderCombo]
Type=0
HeaderFile=jll serverview.h
ImplementationFile=jll serverview.cpp
BaseClass=CComboBox
Filter=W
LastObject=CFolderCombo
VirtualFilter=cWC

