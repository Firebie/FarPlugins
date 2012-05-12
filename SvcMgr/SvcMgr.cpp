/*
    Servelad plugin for FAR Manager
    Copyright (C) 2009 Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

#include "ServiceManager.hpp"

// {9D7C95FA-E67F-41FD-8AF2-B6214EAC9F01}
DEFINE_GUID(MainGuid, 0x9d7c95fa, 0xe67f, 0x41fd, 0x8a, 0xf2, 0xb6, 0x21, 0x4e, 0xac, 0x9f, 0x1);

// {A82E0A2A-63FD-4E2B-A641-A41366A3A6CD}
DEFINE_GUID(MenuGuid, 0xa82e0a2a, 0x63fd, 0x4e2b, 0xa6, 0x41, 0xa4, 0x13, 0x66, 0xa3, 0xa6, 0xcd);

// {5DBACF9B-62B5-4E62-89B9-5EB40944BE86}
DEFINE_GUID(MsgCantStartService, 0x5dbacf9b, 0x62b5, 0x4e62, 0x89, 0xb9, 0x5e, 0xb4, 0x9, 0x44, 0xbe, 0x86);
// {75996F83-B676-45E7-A7E3-67F028F97EB0}
DEFINE_GUID(MsgCantStopService, 0x75996f83, 0xb676, 0x45e7, 0xa7, 0xe3, 0x67, 0xf0, 0x28, 0xf9, 0x7e, 0xb0);



struct PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
const wchar_t PluginName[]      = L"Service Manager";
const wchar_t PluginPrefix[]    = L"svc";
const wchar_t DriversDirName[]  = L"Drivers";
const wchar_t ServicesDirName[] = L"Services";

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *gi)
{
  gi->StructSize=sizeof(gi);
  gi->MinFarVersion=FARMANAGERVERSION;
  gi->Version=MAKEFARVERSION(0,1,1,1,VS_RELEASE);
  gi->Guid=MainGuid;
  gi->Title=L"SvcMgr";
  gi->Description=L"Service manager plugin";
  gi->Author=L"TeaTeam";
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info=*psi;
  FSF=*psi->FSF;
  Info.FSF=&FSF;
}

HANDLE WINAPI OpenW(const struct OpenInfo *oi)
{
  CServiceManager* sm = new CServiceManager();
  return (HANDLE)sm;
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *cpi)
{
  CServiceManager* sm=(CServiceManager*)cpi->hPanel;
  delete sm;
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
  static const wchar_t* MenuStrings[1];
  pi->StructSize=sizeof(PluginInfo);
  pi->Flags=0;
  pi->CommandPrefix=PluginPrefix;
  MenuStrings[0]=PluginName;
  pi->PluginMenu.Guids=&MenuGuid;
  pi->PluginMenu.Strings=MenuStrings;
  pi->PluginMenu.Count=ArraySize(MenuStrings);
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *opi)
{
  static wchar_t PanelTitle[50];
  CServiceManager* sm=(CServiceManager*)opi->hPanel;
  opi->StructSize=sizeof(*opi);
  opi->Flags=OPIF_ADDDOTS|OPIF_SHOWPRESERVECASE|OPIF_SHORTCUT;
  opi->CurDir=NULL;
  if(sm->GetType()==SERVICE_WIN32)
    opi->CurDir=ServicesDirName;
  else if(sm->GetType()==SERVICE_DRIVER)
    opi->CurDir=DriversDirName;

  FSF.snprintf(PanelTitle,ArraySize(PanelTitle)-1,L"%s%s%s",PluginName,opi->CurDir?L": ":L"",opi->CurDir?opi->CurDir:L"");
  opi->PanelTitle=PanelTitle;

  static PanelMode PanelModesArray[10];
  memset(&PanelModesArray, 0, sizeof (PanelModesArray));
  opi->PanelModesArray=PanelModesArray;
  opi->PanelModesNumber=ArraySize(PanelModesArray);
  if(sm->GetType()==0)
  {
    PanelModesArray[0].ColumnTypes=L"N";
    PanelModesArray[0].ColumnWidths=L"0";
    PanelModesArray[0].StatusColumnTypes=L"N";
    PanelModesArray[0].StatusColumnWidths=L"0";
  }
  else
  {
    static const wchar_t* ColumnTitles[]={NULL,L"Status",L"Startup"};
    PanelModesArray[0].ColumnTypes=L"N,C0,C1";
    PanelModesArray[0].ColumnWidths=L"0,8,8";
    PanelModesArray[0].StatusColumnTypes=L"N";
    PanelModesArray[0].StatusColumnWidths=L"0";
    PanelModesArray[0].ColumnTitles=ColumnTitles;
  }
  opi->StartPanelMode=L'0';
}

int WINAPI GetFindDataW(struct GetFindDataInfo *gfdi)
{
  CServiceManager* sm=(CServiceManager*)gfdi->hPanel;

  gfdi->PanelItem   = NULL;
  gfdi->ItemsNumber = 0;

  if (sm->GetType()==0)
  {
    gfdi->PanelItem = (PluginPanelItem *)malloc(sizeof(PluginPanelItem)*2);
    memset(gfdi->PanelItem, 0, sizeof(PluginPanelItem)*2);
    gfdi->ItemsNumber = 2;

    gfdi->PanelItem[0].FileName=DriversDirName;
    gfdi->PanelItem[0].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
    gfdi->PanelItem[1].FileName=ServicesDirName;
    gfdi->PanelItem[1].FileAttributes=FILE_ATTRIBUTE_DIRECTORY;
  }
  else
  {
    sm->RefreshList();

    gfdi->PanelItem = (PluginPanelItem *)malloc(sizeof(PluginPanelItem)*sm->GetCount());
    memset(gfdi->PanelItem, 0, sizeof(PluginPanelItem)*sm->GetCount());
    gfdi->ItemsNumber=sm->GetCount();

    for (size_t i=0; i < gfdi->ItemsNumber; i++)
    {
      SServiceInfo info = (*sm)[i];
      
      gfdi->PanelItem[i].FileName          = info.iDisplayName;
      gfdi->PanelItem[i].AlternateFileName = info.iServiceName;
      
      if (info.iStartType == SERVICE_DISABLED)
        gfdi->PanelItem[i].FileAttributes=FILE_ATTRIBUTE_HIDDEN;
      
      const wchar_t** CustomColumnData=(const wchar_t**)calloc(2,sizeof(const wchar_t*));
      if(CustomColumnData)
      {
        switch(info.iServiceStatusProcess.dwCurrentState)
        {
          case SERVICE_CONTINUE_PENDING:
            CustomColumnData[0]=L"Continue";
            break;
          case SERVICE_PAUSE_PENDING:
            CustomColumnData[0]=L"Pausing";
            break;
          case SERVICE_PAUSED:
            CustomColumnData[0]=L"Paused";
            break;
          case SERVICE_RUNNING:
            CustomColumnData[0]=L"Running";
            break;
          case SERVICE_START_PENDING:
            CustomColumnData[0]=L"Starting";
            break;
          case SERVICE_STOP_PENDING:
            CustomColumnData[0]=L"Stopping";
            break;
          case SERVICE_STOPPED:
            CustomColumnData[0]=L"";
            break;
        }
        
        switch(info.iStartType)
        {
          case SERVICE_AUTO_START:
            CustomColumnData[1]=L"Auto";
            break;
          case SERVICE_BOOT_START:
            CustomColumnData[1]=L"Boot";
            break;
          case SERVICE_DEMAND_START:
            CustomColumnData[1]=L"Manual";
            break;
          case SERVICE_DISABLED:
            CustomColumnData[1]=L"Disabled";
            break;
          case SERVICE_SYSTEM_START:
            CustomColumnData[1]=L"System";
            break;
        }
        gfdi->PanelItem[i].CustomColumnData=CustomColumnData;
        gfdi->PanelItem[i].CustomColumnNumber=2;
      }
      gfdi->PanelItem[i].UserData=i;
    }
  }

  return TRUE;
}

void WINAPI FreeFindDataW(const struct FreeFindDataInfo *ffdi)
{
  for(size_t i=0;i<ffdi->ItemsNumber;i++)
  {
    free((void*)ffdi->PanelItem[i].CustomColumnData);
  }
  free(ffdi->PanelItem);
}

int WINAPI SetDirectoryW(const struct SetDirectoryInfo *sdi)
{
  if (sdi->OpMode & OPM_FIND)
    return FALSE;

  CServiceManager* sm=(CServiceManager*)sdi->hPanel;

  if (!wcscmp(sdi->Dir,L"\\") || !wcscmp(sdi->Dir,L".."))
  {
    sm->Reset();
  }
  else if (!wcscmp(sdi->Dir,ServicesDirName))
  {
    sm->Reset(SERVICE_WIN32);
  }
  else if (!wcscmp(sdi->Dir,DriversDirName))
  {
    sm->Reset(SERVICE_DRIVER);
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}

inline int IsEol(wchar_t x) { return x==L'\r' || x==L'\n'; }

std::wstring& RemoveUnprintableCharacters(std::wstring &strStr)
{
  for (size_t i = 0; i < strStr.size(); ++i)
  {
    wchar_t& c = strStr[i];
    if (IsEol(c))
      c=L' ';
  }
  //FSF.Trim();
  return strStr;
}

bool FormatErrorString(bool Nt, DWORD Code, std::wstring& Str)
{
  bool Result=false;
  LPWSTR lpBuffer=nullptr;
  Result=FormatMessage((Nt?FORMAT_MESSAGE_FROM_HMODULE:FORMAT_MESSAGE_FROM_SYSTEM)|FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS, (Nt?GetModuleHandle(L"ntdll.dll"):nullptr), Code, 0, reinterpret_cast<LPWSTR>(&lpBuffer), 0, nullptr)!=0;
  Str=lpBuffer;
  LocalFree(lpBuffer);
  RemoveUnprintableCharacters(Str);
  return Result;
}

bool GetWin32ErrorString(DWORD LastNtStatus, std::wstring& Str)
{
  return FormatErrorString(false, LastNtStatus, Str);
}

wstring GetTempPath()
{
  DWORD dwSize = GetTempPath(0, NULL);
  wchar_t* pBuf = new wchar_t[dwSize + 1];
  DWORD dwTempSize = ::GetTempPath(dwSize, pBuf);

  wstring sPath = pBuf ? pBuf : L"";

  delete[] pBuf;

  return sPath;
}

wstring GetTempFileName(wstring sExt)
{
  GUID* pGuid = new GUID; 

  CoCreateGuid(pGuid); 

  RPC_WSTR pGuidStr = NULL;
  UuidToString(pGuid, &pGuidStr);
  
  wstring sGuid = (wchar_t*)pGuidStr;
  
  RpcStringFree(&pGuidStr);

  delete pGuid;

  return sGuid + sExt;
}

BOOL ViewServiceInfo(CServiceManager* sm)
{
  PanelInfo pi;
  ZeroMemory(&pi, sizeof(PanelInfo));
  pi.StructSize = sizeof(PanelInfo);
  if (Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi))
  {
    size_t CurItem = pi.CurrentItem;
    if (CurItem > 0)
    {
      size_t Size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, CurItem, 0);
      PluginPanelItem *PPI=(PluginPanelItem*)malloc(Size);
      if (PPI)
      {
        FarGetPluginPanelItem FGPPI = {Size, PPI};
        Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, CurItem, &FGPPI);

        size_t Index = FGPPI.Item->UserData;
        free(PPI);

        if (Index < sm->GetCount())
        {
          CServiceInfo& rServiceInfo = sm->GetServiceInfo(Index);
          std::wstring sInfo = rServiceInfo.GetFullInfo();

          wstring sFileName = GetTempPath() + GetTempFileName(L".txt");
          HANDLE hFile = CreateFile(
            sFileName.c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

          bool bWrite = false;
          if (hFile != INVALID_HANDLE_VALUE)
          {
            DWORD dwWritten = 0;
            bWrite = WriteFile(
              hFile, sInfo.c_str(), sInfo.size() * sizeof(wchar_t), &dwWritten, NULL) != FALSE;

            CloseHandle(hFile);
          }

          if (bWrite)
          {
            //Info.Editor(
            //  sFileName.c_str(), 
            //  rServiceInfo.iDisplayName.c_str(), 
            //  0, 0, -1, -1, 
            //  EF_DELETEONLYFILEONCLOSE | EF_DISABLEHISTORY | EF_DISABLESAVEPOS | EF_ENABLE_F6,
            //  1, 1,
            //  CP_DEFAULT);

            Info.Viewer(
              sFileName.c_str(), 
              rServiceInfo.iDisplayName.c_str(),
              0, 0, -1, -1, 
              VF_DELETEONLYFILEONCLOSE | VF_DISABLEHISTORY | VF_ENABLE_F6,
              CP_DEFAULT);
          }
        }
      }
    }
  }

  return TRUE;
}

BOOL StartService(CServiceManager* sm)
{
  PanelInfo pi;
  ZeroMemory(&pi, sizeof(PanelInfo));
  pi.StructSize = sizeof(PanelInfo);
  if (Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi))
  {
    size_t CurItem = pi.CurrentItem;
    if (CurItem > 0)
    {
      size_t Size = Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, CurItem, 0);
      PluginPanelItem *PPI=(PluginPanelItem*)malloc(Size);
      if (PPI)
      {
        FarGetPluginPanelItem FGPPI = {Size, PPI};
        Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, CurItem, &FGPPI);

        size_t Index = FGPPI.Item->UserData;
        free(PPI);

        if (Index < sm->GetCount())
        {
          bool bStart = sm->StartService(Index);
          if (bStart)
            Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, NULL);
          else
          {
            Info.Message(&MainGuid, &MsgCantStartService,
              FMSG_ERRORTYPE|FMSG_WARNING|FMSG_ALLINONE|FMSG_MB_OK,
              NULL,
              (const wchar_t * const *)L"Error",
              0,0);
          }
        }
      }
    }
  }

  return TRUE;
}

BOOL StopService(CServiceManager* sm)
{
  PanelInfo pi;
  ZeroMemory(&pi, sizeof(PanelInfo));
  pi.StructSize = sizeof(PanelInfo);
  if (Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi))
  {
    size_t CurItem = pi.CurrentItem;
    if (CurItem > 0)
    {
      size_t Size = Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELITEM, CurItem, 0);
      PluginPanelItem *PPI=(PluginPanelItem*)malloc(Size);
      if (PPI)
      {
        FarGetPluginPanelItem FGPPI = {Size, PPI};
        Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, CurItem, &FGPPI);

        size_t Index = FGPPI.Item->UserData;
        free(PPI);

        if (Index < sm->GetCount())
        {
          bool bStop = sm->StopService(Index);
          if (bStop)
            Info.PanelControl(PANEL_ACTIVE, FCTL_REDRAWPANEL, 0, NULL);
          else
          {
            Info.Message(&MainGuid, &MsgCantStopService,
              FMSG_ERRORTYPE|FMSG_WARNING|FMSG_ALLINONE|FMSG_MB_OK,
              NULL,
              (const wchar_t * const *)L"Error",
              0,0);
          }
        }
      }
    }
  }

  return TRUE;
}

int WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *info)
{
  if (info->StructSize < sizeof(*info))
    return FALSE;

  if (info->Rec.EventType != KEY_EVENT)
    return FALSE;

  CServiceManager* sm = (CServiceManager*)info->hPanel;

  if (info->Rec.Event.KeyEvent.wVirtualKeyCode == VK_F3)
    return ViewServiceInfo(sm);
  else if (info->Rec.Event.KeyEvent.wVirtualKeyCode == VK_F5)
    return StartService(sm);
  else if (info->Rec.Event.KeyEvent.wVirtualKeyCode == VK_F8)
    return StopService(sm);

  return FALSE;
}
