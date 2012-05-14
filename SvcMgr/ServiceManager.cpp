/*
    ServiceManager.cpp
    Copyright (C) 2010 zg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include "SvcMgr.h"

#include "ServiceManager.hpp"

typedef BOOL (WINAPI * TQueryServiceConfig2W) (
  SC_HANDLE hService,
  DWORD     dwInfoLevel,
  LPBYTE    lpBuffer,
  DWORD     cbBufSize,
  LPDWORD   pcbBytesNeeded);


CServiceInfo::CServiceInfo(
  const wstring& aServiceName,
  const wstring& aDisplayName,
  const wstring& aDescription,
  const wstring& aPath,
  const wstring& aLoadOrderGroup,
  const vector<wstring>& aDependencies,
  DWORD aStartType,
  DWORD aErrorControl,
  DWORD aTagId,
  const wstring& aServiceStartName,
  SERVICE_STATUS_PROCESS aServiceStatusProcess)
  : iServiceName(aServiceName),
  iDisplayName(aDisplayName),
  iDescription(aDescription),
  iPath(aPath),
  iLoadOrderGroup(aLoadOrderGroup),
  iDependencies(aDependencies),
  iStartType(aStartType),
  iErrorControl(aErrorControl),
  iTagId(aTagId),
  iServiceStartName(aServiceStartName),
  iServiceStatusProcess(aServiceStatusProcess)
{
}

wstring CServiceInfo::GetServiceTypeS()
{
  wstring sType;
  DWORD dwServiceType = iServiceStatusProcess.dwServiceType;

  if (dwServiceType == SERVICE_FILE_SYSTEM_DRIVER)
    sType = L"File system driver";
  else if (dwServiceType == SERVICE_KERNEL_DRIVER)
    sType = L"Device driver";
  else if (dwServiceType == SERVICE_WIN32_OWN_PROCESS)
    sType = L"Service. Runs in its own process.";
  else if (dwServiceType == SERVICE_WIN32_SHARE_PROCESS)
    sType = L"Service. Shares a process with other services.";
  else if (dwServiceType == SERVICE_INTERACTIVE_PROCESS)
    sType = L"Service. Can interact with the desktop.";

  return sType;
}

wstring CServiceInfo::GetStartTypeS()
{
  wstring sStartType;
  
  switch (iStartType)
  {
  case SERVICE_BOOT_START:
    sStartType = L"Boot";
    break;
  case SERVICE_SYSTEM_START:
    sStartType = L"System";
    break;
  case SERVICE_AUTO_START:
    sStartType = L"Auto";
    break;
  case SERVICE_DEMAND_START:
    sStartType = L"Manual";
    break;
  case SERVICE_DISABLED:
    sStartType = L"Disabled";
    break;
  }

  return sStartType;
}

wstring CServiceInfo::GetCurrentStateS()
{
  wstring sCurrentState;

  switch (iServiceStatusProcess.dwCurrentState)
  {
  case SERVICE_CONTINUE_PENDING:
    sCurrentState = L"Continue";
    break;
  case SERVICE_PAUSE_PENDING:
    sCurrentState = L"Pausing";
    break;
  case SERVICE_PAUSED:
    sCurrentState = L"Paused";
    break;
  case SERVICE_RUNNING:
    sCurrentState = L"Running";
    break;
  case SERVICE_START_PENDING:
    sCurrentState = L"Starting";
    break;
  case SERVICE_STOP_PENDING:
    sCurrentState = L"Stopping";
    break;
  case SERVICE_STOPPED:
    sCurrentState = L"Not running";
    break;
  }

  return sCurrentState;
}


wstring CServiceInfo::GetErrorControlTypeS()
{
  wstring sErrorControlType;

  switch (iErrorControl)
  {
  case SERVICE_ERROR_CRITICAL:
    sErrorControlType = L"Critical";
    break;
  case SERVICE_ERROR_IGNORE:
    sErrorControlType = L"Ignore";
    break;
  case SERVICE_ERROR_NORMAL:
    sErrorControlType = L"Normal";
    break;
  case SERVICE_ERROR_SEVERE:
    sErrorControlType = L"Severe";
    break;
  }

  return sErrorControlType;
}

wstring CServiceInfo::GetFullInfo()
{
  wstring sInfo;

  sInfo =  L"Name:                   " + iServiceName + L"\n";
  sInfo += L"Display Name:           " + iDisplayName + L"\n\n";
  sInfo += L"Description:            " + iDescription + L"\n\n";
  sInfo += L"Type:                   " + GetServiceTypeS() + L"\n\n";
  sInfo += L"Start type:             " + GetStartTypeS() + L"\n";
  sInfo += L"Status:                 " + GetCurrentStateS() + L"\n\n";
  sInfo += L"Error Control type:     " + GetErrorControlTypeS() + L"\n\n";
  sInfo += L"Log on as:              " + iServiceStartName + L"\n\n";

  if (iDependencies.empty())
  {
    sInfo += L"Depends on:             \n\n";
  }
  else
  {
    for (vector<wstring>::iterator it = iDependencies.begin();
      it != iDependencies.end();
      ++it)
    {
      if (it == iDependencies.begin())
        sInfo += L"Depends on:             " + *it + L"\n";
      else
        sInfo += L"                        " + *it + L"\n";
    }
    sInfo += L"\n";
  }

  int nPathSize = iPath.length() + 1 + 2;
  wchar_t* pBuf = new wchar_t[nPathSize];
  wcscpy_s(pBuf, nPathSize, iPath.c_str());
  wstring sPath = FSF.QuoteSpaceOnly(pBuf);
  delete[] pBuf;
  pBuf = NULL;
  
  sInfo += L"Binary Path:            " + sPath + L"\n";

  return sInfo;
}


SServiceInfo CServiceInfo::Data(void) const
{
  SServiceInfo info = 
  {
    iServiceName.c_str(),
    iDisplayName.c_str(),
    iDescription.c_str(),
    iPath.c_str(),
    iLoadOrderGroup.c_str(),
    iStartType,
    iErrorControl,
    iTagId,
    iServiceStatusProcess
  };
  
  return info;
}

vector<wstring> ConvertToVector(wchar_t* pStr)
{
  vector<wstring> vItems;
  while (true)
  {
    wstring s = pStr ? pStr : L"";
    if (!s.empty())
      vItems.push_back(s);
    else
      break;

    pStr += s.size() + 1;
  }
  return vItems;
}

bool CServiceList::Fill(DWORD aServiceType)
{
  Clear();
  
  if (aServiceType == 0)
    return true;

  DWORD     cbBytesNeeded      = 0;
  DWORD     dwServicesReturned = 0;
  DWORD     dwResumeHandle     = 0;
  SC_HANDLE handle             = NULL;
  bool      result             = false;
  char*     serviceConfigData  = NULL;

  TQueryServiceConfig2W pQueryServiceConfig2 = NULL;
  HMODULE hAdvApi = LoadLibrary(L"Advapi32.dll");
  if (hAdvApi)
  {
    pQueryServiceConfig2 = (TQueryServiceConfig2W)GetProcAddress(hAdvApi, "QueryServiceConfig2W");
  }

  BOOL ret = EnumServicesStatusEx(
    iSCManager,
    SC_ENUM_PROCESS_INFO,
    aServiceType,
    SERVICE_STATE_ALL,
    NULL,
    0,
    &cbBytesNeeded,
    &dwServicesReturned,
    &dwResumeHandle,
    NULL);

  ENUM_SERVICE_STATUS_PROCESS* pServices = NULL;

  if (!ret && GetLastError() == ERROR_MORE_DATA)
  {
    try
    {
      pServices = (ENUM_SERVICE_STATUS_PROCESS*)new BYTE[cbBytesNeeded];
      ret = EnumServicesStatusEx(
        iSCManager,
        SC_ENUM_PROCESS_INFO,
        aServiceType,
        SERVICE_STATE_ALL,
        (BYTE*)pServices,
        cbBytesNeeded,
        &cbBytesNeeded,
        &dwServicesReturned,
        &dwResumeHandle,
        NULL);

      if (ret)
      {
        for (size_t nService = 0; nService < dwServicesReturned; ++nService)
        {
          handle = OpenService(iSCManager, pServices[nService].lpServiceName, SERVICE_QUERY_CONFIG);
          if(!handle) 
            throw std::exception();
          
          DWORD needed = 0;
          ret = QueryServiceConfig(handle, NULL, 0, &needed);
          if (ret || GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
          {
            throw std::exception();
          }

          serviceConfigData = new char[needed];
          QUERY_SERVICE_CONFIG* serviceConfig = (QUERY_SERVICE_CONFIG*)serviceConfigData;
          
          if (!QueryServiceConfig(handle, serviceConfig, needed, &needed)) 
            throw std::exception();

          std::wstring sDescription;
          if (pQueryServiceConfig2)
          {
            DWORD dwBytesNeeded;
            ret = pQueryServiceConfig2(handle, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwBytesNeeded);
            if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
              BYTE* pBuffer = new BYTE[dwBytesNeeded];

              ret = pQueryServiceConfig2(handle, SERVICE_CONFIG_DESCRIPTION, pBuffer, dwBytesNeeded, &dwBytesNeeded);
              if (ret)
              {
                SERVICE_DESCRIPTION* pServiceDescr = (SERVICE_DESCRIPTION*)pBuffer;
                sDescription = pServiceDescr->lpDescription ? pServiceDescr->lpDescription : L"";
              }

              delete[] pBuffer;
              pBuffer = NULL;
            }
          }
          
          CloseServiceHandle(handle);
          handle = NULL;

          CServiceInfo si(
            pServices[nService].lpServiceName,
            pServices[nService].lpDisplayName,
            sDescription,
            serviceConfig->lpBinaryPathName,
            serviceConfig->lpLoadOrderGroup,
            ConvertToVector(serviceConfig->lpDependencies),
            serviceConfig->dwStartType,
            serviceConfig->dwErrorControl,
            serviceConfig->dwTagId,
            serviceConfig->lpServiceStartName ? serviceConfig->lpServiceStartName : L"",
            pServices[nService].ServiceStatusProcess);

          iServicesInfo.push_back(si);
          delete[] serviceConfigData;
          serviceConfigData = NULL;
        }
        iServiceCount = dwServicesReturned;
        result = true;
      }
    }
    catch(...)
    {
      Clear();
    }
  }

  delete[] pServices;
  pServices = NULL;

  if (hAdvApi)
  {
    FreeLibrary(hAdvApi);
    hAdvApi = NULL;
  }

  if(handle) 
  {
    CloseServiceHandle(handle);
    handle = NULL;
  }

  if(serviceConfigData)
  {
    delete[] serviceConfigData;
    serviceConfigData = NULL;
  }
  return result;
}

void CServiceList::Clear(void)
{
  iServicesInfo.clear();
  
  iServiceCount = 0;
}

CServiceList::CServiceList(const wstring& aRemoteMachine)
  : iRemoteMachine(aRemoteMachine),
  iSCManager(NULL),
  iServiceCount(0)
{
}

bool CServiceList::Init()
{
  iSCManager = OpenSCManager(
    iRemoteMachine.c_str(), SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);

  return iSCManager != NULL;
}

CServiceList::~CServiceList()
{
  Clear();
  if(iSCManager) 
  {
    CloseServiceHandle(iSCManager);
    iSCManager = NULL;
  }
}

bool CServiceList::StartService(size_t anIndex)
{
  SServiceInfo SI = (*this)[anIndex];

  SC_HANDLE hService = OpenService(
    iSCManager,
    SI.iServiceName,
    GENERIC_EXECUTE);

  if (hService == NULL)
    return false;
  
  bool bStart = ::StartService(hService, 0, NULL) != FALSE;

  CloseServiceHandle(hService);
  hService = NULL;

  return bStart;
}

bool CServiceList::StopService(size_t anIndex)
{
  SServiceInfo SI = (*this)[anIndex];

  SC_HANDLE hService = OpenService(
    iSCManager,
    SI.iServiceName,
    SERVICE_STOP);

  if (hService == NULL)
    return false;

  SERVICE_STATUS ss;
  bool bStop = ::ControlService(hService, SERVICE_CONTROL_STOP, &ss) != FALSE;

  CloseServiceHandle(hService);
  hService = NULL;

  return bStop;
}

bool CServiceList::PauseService(size_t anIndex)
{
  SServiceInfo SI = (*this)[anIndex];

  SC_HANDLE hService = OpenService(
    iSCManager,
    SI.iServiceName,
    SERVICE_PAUSE_CONTINUE);

  if (hService == NULL)
    return false;

  SERVICE_STATUS ss;
  bool bStop = ::ControlService(hService, SERVICE_CONTROL_PAUSE, &ss) != FALSE;

  CloseServiceHandle(hService);
  hService = NULL;

  return bStop;
}

bool CServiceList::SetServiceStartupType(size_t anIndex, DWORD anStartType)
{
  SServiceInfo SI = (*this)[anIndex];

  SC_HANDLE hService = OpenService(
    iSCManager,
    SI.iServiceName,
    SERVICE_CHANGE_CONFIG);

  if (hService == NULL)
    return false;

  SERVICE_STATUS ss;
  BOOL bChange = ::ChangeServiceConfig(
    hService, 
    SERVICE_NO_CHANGE,
    anStartType,
    SERVICE_NO_CHANGE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL);

  CloseServiceHandle(hService);
  hService = NULL;

  return bChange != FALSE;
}

bool CServiceList::QueryServiceStatus(size_t anIndex, SERVICE_STATUS_PROCESS& QueryServiceStatus)
{
  SServiceInfo SI = (*this)[anIndex];

  SC_HANDLE hService = OpenService(
    iSCManager,
    SI.iServiceName,
    SERVICE_QUERY_STATUS);

  if (hService == NULL)
    return false;

  DWORD nSizeNeeded = 0;
  BOOL bQuerySize = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, NULL, 0, &nSizeNeeded);
  BOOL bQuery = FALSE;
  if (!bQuerySize && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    BYTE* pBuf = new BYTE[nSizeNeeded];
    
    bQuery = QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, pBuf, nSizeNeeded, &nSizeNeeded);
    if (bQuery)
      QueryServiceStatus = *(SERVICE_STATUS_PROCESS*)pBuf;

    delete[] pBuf;
  }


  CloseServiceHandle(hService);
  hService = NULL;

  return bQuery != FALSE;
}

bool CServiceManager::StartService(size_t anIndex)
{
  if (anIndex >= GetCount())
    return false;

  return iServiceList.StartService(anIndex);
}

bool CServiceManager::StopService(size_t anIndex)
{
  if (anIndex >= GetCount())
    return false;

  return iServiceList.StopService(anIndex);
}

bool CServiceManager::PauseService(size_t anIndex)
{
  if (anIndex >= GetCount())
    return false;

  return iServiceList.PauseService(anIndex);
}

bool CServiceManager::SetServiceStartupType(size_t anIndex, DWORD anStartType)
{
  if (anIndex >= GetCount())
    return false;

  return iServiceList.SetServiceStartupType(anIndex, anStartType);
}

bool CServiceManager::QueryServiceStatus(size_t anIndex, SERVICE_STATUS_PROCESS& QueryServiceStatus)
{
  if (anIndex >= GetCount())
    return false;

  return iServiceList.QueryServiceStatus(anIndex, QueryServiceStatus);
}

