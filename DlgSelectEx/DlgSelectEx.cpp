#include <plugin.hpp>
#include <initguid.h>

#include "guid.hpp"
#include "version.hpp"

static PluginStartupInfo    Info;
static FARSTANDARDFUNCTIONS FSF;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
  Info->StructSize=sizeof(GlobalInfo);
  Info->MinFarVersion=FARMANAGERVERSION;
  Info->Version=PLUGIN_VERSION;
  Info->Guid=MainGuid;
  Info->Title=PLUGIN_NAME;
  Info->Description=PLUGIN_DESC;
  Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
  Info     = *psi;
  FSF      = *psi->FSF;
  Info.FSF = &FSF;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
  Info->StructSize = sizeof(*Info);
  Info->Flags      = PF_PRELOAD;
}

size_t GetTextLen(HANDLE hDlg, intptr_t Id)
{
  size_t nLen = (size_t)Info.SendDlgMessage(hDlg, DM_GETTEXT, Id, nullptr);

  if (nLen > 0)
  {
    FarDialogItemData ID = {sizeof(ID)};
    ID.PtrLength = nLen + 1;
    ID.PtrData   = new wchar_t[nLen + 1];
    Info.SendDlgMessage(hDlg, DM_GETTEXT, Id, &ID);
    wchar_t* str = ID.PtrData;
    str[nLen] = 0;
    nLen = wcslen(Info.FSF->Trim(str));

    delete[] ID.PtrData;
  }

  return nLen;
}

void ProcessFocus(intptr_t Msg, HANDLE hDlg, intptr_t Id)
{
  if (Msg == DN_GOTFOCUS || Msg == DN_KILLFOCUS)
  {
    FarDialogItem DI;
    Info.SendDlgMessage(hDlg, DM_GETDLGITEMSHORT, Id, &DI);
    if ((DI.Type == DI_EDIT || DI.Type == DI_PSWEDIT || DI.Type == DI_FIXEDIT)
      && (DI.Flags & DIF_EDITOR) == 0)
    {
      EditorSelect ES = {sizeof(ES), 0, 0, 0, 0, 0};

      size_t nLen = 0;
      if (Msg == DN_GOTFOCUS)
      {
        if (DI.Type == DI_FIXEDIT)
          nLen = GetTextLen(hDlg, Id);
        else
          nLen = (size_t)Info.SendDlgMessage(hDlg, DM_GETTEXT, Id, nullptr);

        ES.BlockType   = BTYPE_STREAM;
        ES.BlockWidth  = nLen;
        ES.BlockHeight = 1;

        Info.SendDlgMessage(hDlg, DM_SETSELECTION, Id, &ES);

        COORD Coord = { (SHORT)nLen, 0 };
        Info.SendDlgMessage(hDlg, DM_SETCURSORPOS, Id, &Coord);
      }
      else
      {
        Info.SendDlgMessage(hDlg, DM_SETSELECTION, Id, &ES);
      }
    }
  }
}

intptr_t WINAPI ProcessDialogEventW(const struct ProcessDialogEventInfo *Info)
{
  if (Info->Event == DE_DEFDLGPROCINIT)
    ProcessFocus(Info->Param->Msg, Info->Param->hDlg, Info->Param->Param1);

  return 0;
}
