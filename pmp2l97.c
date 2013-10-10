/* Small hardware monitor for the Asus P2L97 mainboard
 *   OS/2-EMX
 *
 *   bjorn@mork.no
 */

#define INCL_WIN
#include <os2.h>
#include "version.h"
#include "p2l97.h"
#include "pmp2l97.h"

#define NA  "n/a"

HAB hab;
HWND hpopup;   /* popup menu handle */
HPOINTER hicon;   /* icon handle */
int options = OPT_CELSIUS + OPT_TITLEBAR;   /* temperatures in centigrade, show titlebar */
int update  = 2000;                               /* update interval in milliseconds */
int sliderrange;

char *fanprint(char *buf, short tick, short fandiv)
{
  if (tick == 255) strcpy(buf,NA);
  else sprintf(buf, "%d rpm", LM78_RPM(tick, fandiv));
  return buf;
}

void UpdateDisplay(HWND hwnd)
{
  char buf[64];
  int lread, chipid;

  WinSetDlgItemText(hwnd, IDD_FAN_CHASSIS, fanprint(buf, LM78_READ(LM78_FAN1), LM78_FANDIV1));
  WinSetDlgItemText(hwnd, IDD_FAN_CPU, fanprint(buf, LM78_READ(LM78_FAN2), LM78_FANDIV2));
  WinSetDlgItemText(hwnd, IDD_FAN_POWER, fanprint(buf, LM78_READ(LM78_FAN3), LM78_FANDIV3));
  WinSetDlgItemText(hwnd, IDD_TEMP_CPU1, NA);  /* CPU temperature measurements are disabled */
  WinSetDlgItemText(hwnd, IDD_TEMP_CPU2, NA);  /* CPU temperature measurements are disabled */

  LM78_WRITE(LM78_BANKSEL, LM78_BANK0);
  lread = LM78_READ(LM78_VENDOR);
  if (lread == 0x5c)
     {
     LM78_WRITE(LM78_BANKSEL, (LM78_BANK0 & 0x7f));
     lread = LM78_READ(LM78_VENDOR);
     if (lread == 0xa3)
        {
//        LM78_WRITE(LM78_BANKSEL, LM78_BANK0);
        chipid = LM78_READ(LM78_CHIPID);
//        printf("LM78_CHIPD = 0x%02x\n", lread);
//        lread = LM78_READ(LM78_DIODE);
//        printf("LM78_DIODE = 0x%02x\n", lread);

        LM78_WRITE(LM78_BANKSEL, LM78_BANK1);
        lread = LM78_READ(LM78_TEMP_CFG);
        //printf("LM78_TEMP_CFG1 = 0x%02x\n", lread);
        if ((lread & 0x01) == 0)
           {
           int i;

           i = LM78_READ(LM78_EXT_TEMP1);
           //printf("LM78_EXT_TEMP1 = 0x%02x\n", i);
           if (i > 127)
              i = (256 - i)*(-1);
           if (!(options & OPT_CELSIUS))
              i = i * 9/5 +32;
           sprintf(buf, "%d ø%c", i, (options & OPT_CELSIUS) ? 'C' : 'F');
           WinSetDlgItemText(hwnd, IDD_TEMP_CPU1, buf);
           }

        LM78_WRITE(LM78_BANKSEL, LM78_BANK2);
        lread = LM78_READ(LM78_TEMP_CFG);
        if ((lread & 0x01) == 0)
           {
           int i;

           i = LM78_READ(LM78_EXT_TEMP2);
           //printf("LM78_EXT_TEMP2 = 0x%02x\n", i);
           if (i > 127)
              i = (256 - i)*(-1);
           if (!(options & OPT_CELSIUS))
              i = i * 9/5 +32;
           sprintf(buf, "%d ø%c", i, (options & OPT_CELSIUS) ? 'C' : 'F');
           if (i < 0)
              WinSetDlgItemText(hwnd, IDD_TEMP_CPU2, NA);
           else
              WinSetDlgItemText(hwnd, IDD_TEMP_CPU2, buf);
           }
        LM78_WRITE(LM78_BANKSEL, LM78_BANK0);
        }
     }

  if (options & OPT_CELSIUS)
    sprintf(buf, "%d øC", LM78_READ(LM78_TEMP));
  else
    sprintf(buf, "%d øF", LM78_READ(LM78_TEMP)*9/5+32);
  WinSetDlgItemText(hwnd, IDD_TEMP_MB, buf);
  sprintf(buf, "%6.2fV", P2L97_VCORE);
  WinSetDlgItemText(hwnd, IDD_V_VCORE, buf);
  sprintf(buf, "%6.2fV", P2L97_3_3V);
  WinSetDlgItemText(hwnd, IDD_V_3_3V, buf);
  sprintf(buf, "%6.2fV", P2L97_5V);
  WinSetDlgItemText(hwnd, IDD_V_5V, buf);
  sprintf(buf, "%6.2fV", P2L97_12V);
  WinSetDlgItemText(hwnd, IDD_V_12V, buf);
  if (chipid == 0x21)
     {
     float i;

     i = LM78_READ(LM78_IN5_NEG) * -0.3438;
     sprintf(buf, "%6.2fV", i);
     WinSetDlgItemText(hwnd, IDD_V_M12V, buf);
     i = LM78_READ(LM78_IN6_NEG) * -0.0858;
     sprintf(buf, "%6.2fV", i);
     WinSetDlgItemText(hwnd, IDD_V_M5V, buf);
     }
  else
     {
     sprintf(buf, "%6.2fV", P2L97_M12V);
     WinSetDlgItemText(hwnd, IDD_V_M12V, buf);
     sprintf(buf, "%6.2fV", P2L97_M5V);
     WinSetDlgItemText(hwnd, IDD_V_M5V, buf);
     }
}

static MRESULT EXPENTRY SetupDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
  {
  char buf[64];

  switch (msg)
     {
     case WM_INITDLG:
        {
        if (options & OPT_CELSIUS)
           WinCheckButton(hwnd, IDD_DEG_C, 1);
        else
           WinCheckButton(hwnd,IDD_DEG_F,1);
        sprintf(buf, "%d.%d seconds", update/1000, update%1000/100);
        WinSetDlgItemText(hwnd, IDD_REFRESH_TEXT, buf);
        sliderrange = SHORT2FROMMR(WinSendDlgItemMsg(hwnd, IDD_REFRESH, SLM_QUERYSLIDERINFO,
                                  MPFROM2SHORT(SMA_SLIDERARMPOSITION,SMA_RANGEVALUE),MPVOID));
        WinSendDlgItemMsg(hwnd, IDD_REFRESH, SLM_SETSLIDERINFO,
                          MPFROM2SHORT(SMA_SLIDERARMPOSITION, SMA_INCREMENTVALUE), MPFROMSHORT(update/500-1));
        return (MRESULT)0;
        }

     case WM_CONTROL:
        {
        switch (SHORT1FROMMP(mp1))
           {
           case IDD_REFRESH:
              switch (SHORT2FROMMP(mp1))
                 {
                 case SLN_CHANGE:
                 case SLN_SLIDERTRACK:
                    update = SHORT1FROMMP(mp2)*5000/sliderrange+500;
                    sprintf(buf, "%d.%d seconds", update/1000, update%1000/100);
                    WinSetDlgItemText(hwnd, IDD_REFRESH_TEXT, buf);
                    break;
                 }
           break;
           }
        }
      break;

     case WM_DESTROY:
        {
        if (WinQueryButtonCheckstate(hwnd, IDD_DEG_C))
           options |= OPT_CELSIUS;
        else
           options &= ~OPT_CELSIUS;
        break;
        }
     }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
  }

void ShowTitleBar(HWND hwnd, BOOL show, BOOL resize)
  {
  HWND t;
  SWP size;
  int delta;

  WinCheckMenuItem(hpopup, IDM_TITLEBAR, show);
  t = WinWindowFromID(hwnd, FID_TITLEBAR);
  if (resize)
     {
     WinQueryWindowPos(t, &size);
     delta = size.cy;
     WinQueryWindowPos(hwnd, &size);
     if (show)
        size.cy += delta;
     else
        size.cy -= delta;
     WinSetWindowPos(hwnd, 0, 0, 0, size.cx, size.cy, SWP_SIZE);
     }
  WinShowWindow(t, show);
  WinShowWindow(WinWindowFromID(hwnd, FID_SYSMENU), show);
  WinShowWindow(WinWindowFromID(hwnd, FID_MINMAX), show);
  }

void InitWindow(HWND hwnd)
  {
  long x,y;

  WinRestoreWindowPos(AppName, "WindowPos", hwnd);
  y = sizeof(x);
  if (PrfQueryProfileData(HINI_USERPROFILE, AppName, "Options", &x, &y))
     options = x;
  y = sizeof(x);
  if (PrfQueryProfileData(HINI_USERPROFILE, AppName, "Refresh", &x, &y))
     update = x;
  WinSetAccelTable(hab, WinLoadAccelTable(hab, 0, ID_PMP2L97), hwnd);
  hpopup = WinLoadMenu(hwnd, 0, ID_PMP2L97);
  WinCheckMenuItem(hpopup, IDM_ONTOP, (options & OPT_ONTOP));
  UpdateDisplay(hwnd);
  WinStartTimer(hab, hwnd, ID_PMP2L97, update);
  WinSendMsg(hwnd, WM_SETICON, MPFROMP(hicon), MPVOID); /* setting the icon. Is there an easier way? */
  ShowTitleBar(hwnd, (options & OPT_TITLEBAR), FALSE);   /* don't resize, since the window size was also restored */
  }

void Setup(HWND hwnd)
  {
  int sav_update, sav_options;

  sav_update = update;
  sav_options = options;
  if (WinDlgBox(HWND_DESKTOP, hwnd, SetupDlgProc, 0, ID_SETUP,NULL) == DID_OK)
     WinStartTimer(hab, hwnd, ID_PMP2L97, update);
  else
     {
     update = sav_update;
     options = sav_options;
     }
  }

static MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  switch (msg)
     {
     case WM_INITDLG:
        {
        LM78_INIT;
        if (!LM78_EXIST)
           {
           WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, "Cannot find an LM78", NULL, 0, MB_ERROR | MB_OK);
           WinDismissDlg(hwnd, -1);
           }
        else
           InitWindow(hwnd);
        return (MRESULT)0;
        }

     case WM_COMMAND:
        {
        switch (SHORT1FROMMP(mp1))
           {
           case IDM_ONTOP:
              options ^= OPT_ONTOP;
              WinCheckMenuItem(hpopup, IDM_ONTOP, (options & OPT_ONTOP));
              return (MRESULT)0;

           case IDM_TITLEBAR:
              options ^= OPT_TITLEBAR;
              ShowTitleBar(hwnd, (options & OPT_TITLEBAR), TRUE);
              return (MRESULT)0;

           case IDM_SETUP:
              Setup(hwnd);
              return (MRESULT)0;

           case IDM_ABOUT:
              WinDlgBox(HWND_DESKTOP, hwnd, WinDefDlgProc, 0, ID_ABOUT, NULL);
              return (MRESULT)0;

           case IDM_EXIT:
              WinSendMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
              return (MRESULT)0;
           }
        }
        break;

     case WM_ACTIVATE:
        {
        ShowTitleBar(hwnd, (options & OPT_TITLEBAR), FALSE);  /* avoid activating a disabled titlebar */
        }
        break;

     case WM_TIMER:
        {
        UpdateDisplay(hwnd);
        if (options & OPT_ONTOP)             /* is there a better way to do this? */
           WinSetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER);
        }
        break;

     case WM_BUTTON1DOWN:
        {
        WinSendMsg(hwnd, WM_TRACKFRAME, MPFROMSHORT(TF_MOVE), MPVOID);
        }
        break;

     case WM_BUTTON1UP:
        {
        WinSendMsg(hwnd, WM_TRACKFRAME, MPVOID, MPVOID);
        }
        break;

     case WM_BUTTON2DOWN:
        {
        WinPopupMenu(hwnd, hwnd, hpopup, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), 0,
                    PU_HCONSTRAIN |
                    PU_VCONSTRAIN |
                    PU_MOUSEBUTTON1 |
                    PU_MOUSEBUTTON2 |
                    PU_KEYBOARD);
        return (MRESULT)0;
        }

     case WM_CLOSE:
        {
        WinDestroyWindow(hpopup);
        WinStoreWindowPos(AppName, "WindowPos", hwnd);
        PrfWriteProfileData(HINI_USERPROFILE, AppName, "Options", &options, sizeof(options));
        PrfWriteProfileData(HINI_USERPROFILE, AppName, "Refresh", &update, sizeof(update));
        }
     } // switch
  return WinDefDlgProc(hwnd,msg,mp1,mp2);
  }

int main (int argc, char **argv)
  {
  HMQ hmq;
  QMSG qmsg;
  int result;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue(hab, 0);
  hicon = WinLoadFileIcon(argv[0], FALSE);   /* how do you load an icon from a resource? */
  result = WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, ClientWndProc, 0, ID_PMP2L97, NULL);
  WinFreeFileIcon(hicon);
  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  return result;
  }
