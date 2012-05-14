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

#ifndef __SERVICEMANAGER_HPP__
#define __SERVICEMANAGER_HPP__

struct SServiceInfo
{
  const wchar_t* iServiceName;
  const wchar_t* iDisplayName;
  const wchar_t* iDescription;
  const wchar_t* iPath;
  const wchar_t* iLoadOrderGroup;
  //const wchar_t* iDependencies;
  DWORD iStartType;
  DWORD iErrorControl;
  DWORD iTagId;
  SERVICE_STATUS_PROCESS iServiceStatusProcess;
};

class CServiceInfo
{
public:
  wstring iServiceName;
  wstring iDisplayName;
  wstring iDescription;
  wstring iPath;
  wstring iLoadOrderGroup;
  vector<wstring> iDependencies;
  DWORD iStartType;
  DWORD iErrorControl;
  DWORD iTagId;
  wstring iServiceStartName;
  SERVICE_STATUS_PROCESS iServiceStatusProcess;

  wstring GetServiceTypeS();
  wstring GetStartTypeS();
  wstring GetCurrentStateS();
  wstring GetErrorControlTypeS();
  wstring GetFullInfo();

public:
  CServiceInfo(
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
    SERVICE_STATUS_PROCESS aServiceStatusProcess);

    SServiceInfo Data() const;
};

class CServiceList
{
private:
  wstring iRemoteMachine;
  SC_HANDLE iSCManager;
  DWORD iServiceCount;
  std::vector<CServiceInfo> iServicesInfo;
private:
  CServiceList(const CServiceList& copy);
public:
  CServiceList(const std::wstring& aRemoteMachine=L"");
  ~CServiceList();
  
  bool Init();

public:
  bool ManagerStatus(void) const { return iSCManager != NULL; }
  bool Fill(DWORD aServiceType);
  void Clear(void);
  DWORD ServiceCount(void)const { return iServiceCount; }
  SServiceInfo operator[](size_t anIndex) const { return iServicesInfo[anIndex].Data(); }

  CServiceInfo& GetServiceInfo(size_t anIndex) { return iServicesInfo[anIndex]; }

  bool StartService(size_t anIndex);
  bool StopService(size_t anIndex);
  bool SetServiceStartupType(size_t anIndex, DWORD anStartType);
  bool QueryServiceStatus(size_t anIndex, SERVICE_STATUS_PROCESS& QueryServiceStatus);
};

class CServiceManager
{
  private:
    CServiceList iServiceList;
    DWORD iServiceType;
  public:
    CServiceManager(const wstring& aRemoteMachine = L"")
      : iServiceList(aRemoteMachine),
      iServiceType(0) 
    {}

    bool Init()
    {
      return iServiceList.Init();
    }

    bool ManagerStatus() const { return iServiceList.ManagerStatus(); }
    void Reset(DWORD aServiceType=0) { iServiceType = aServiceType; Clear(); }

    bool StartService(size_t anIndex);
    bool StopService(size_t  anIndex);
    bool SetServiceStartupType(size_t anIndex, DWORD anStartType);
    bool QueryServiceStatus(size_t anIndex, SERVICE_STATUS_PROCESS& QueryServiceStatus);

    DWORD GetType()  const { return iServiceType; }
    DWORD GetCount() const { return iServiceList.ServiceCount(); }
    SServiceInfo operator[](size_t anIndex) const { return iServiceList[anIndex]; }
    
    CServiceInfo& GetServiceInfo(size_t anIndex) { return iServiceList.GetServiceInfo(anIndex); }

    bool RefreshList(void) { return iServiceList.Fill(iServiceType); }
  private:
    void Clear(void) { iServiceList.Clear(); }
  private:
    CServiceManager(const CServiceManager &copy);
};

#define ArraySize(a) (sizeof(a)/sizeof(a[0]))

#endif
