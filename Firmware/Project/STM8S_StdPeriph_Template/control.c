//----------------------------------------------------------------------------

//DSS-90/DFS-90
//������ ���������� �������� � ������� �������

//��� ����������� ����������� ������������ PID-��������. ����� PID
//������� � ���������������� ����� (��� ��� ����� �������� ���������������).
//����� ������ ���������� ������������������ ������� �������. �����������
//������������ ��������� �����������, ���� ����������� ����������
//������������� �������������.
//��� ����� ������� ���������� PID ����������� ������������ �����������
//������������ ������������. ������������ ����� ���� ����������
//�������� �����������, ��� ���� ���� ������ ��������.
//�������� ����������� ������������ ���������� Tset, �� �� ������
//� ������ ���������� ����������� Tget ����������� ����������� ��������
//�������� ����������� Pact. ������ �������� ���������� ��������� Plim.
//������� ����������� ����������������� ���������� ��������� �����������,
//����� PID. ��� ����� ����� ���������� PidH_Ki = 0. � ����� ������
//�������� ������������ ���������� Pset.

//��� DFS:
//��� ����������� ������� �������� ������ ����������� �����
//������������ PID-��������. �������� �������� ����������� ������������
//���������� Vset, �� �� ������ � ������ ���������� �������� Vget
//����������� ����������� �������� PWM ����������� Wact.
//������� ����������� ����������������� ���������� PWM �����������,
//����� PID. ��� ����� ����� ���������� PidF_Ki = 0. � ����� ������
//PWM ������������ ���������� Wset.

//������������ PID � �������� ������������ �������� ��� �� �������
//����������� � EEPROM � ����������� ������ ������ ��� ��� ������ ������.

//������ ��������� ���� ������ ��� ������������� ����� �� ���������.
//��� ������ ����� ���� ������ � ������� ������� Control_GetError().

//----------------------------------------------------------------------------
#include "stm8s.h"
#include "control.h"
//------------------------------ ���������: ----------------------------------

#define SPS 4//(1000 / TSAMPLE) //���������� �������� ����������� � �������

#define P_LIM_NOM     47  //����������� �������� �����������, x0.1%
#define PIDH_KP_NOM   20  //����������� ����������� PidH_Kp
#define PIDH_KI_NOM   22  //����������� ����������� PidH_Ki
#define PIDH_KD_NOM   20  //����������� ����������� PidH_Kd

#define OPT_ERHTM_NOM  3000  //�������� ����������� �����������, ��
#define OPT_ERHDT_NOM   350  //������������ ���������� �����������, x0.1�C
#define OPT_ERHVT_NOM    70  //����. �������� ����� �����������, x0.1�C/�

#define PIDF_KP_NOM      47  //����������� ����������� PidF_Kp
#define PIDF_KI_NOM      70  //����������� ����������� PidF_Ki
#define PIDF_KD_NOM      50  //����������� ����������� PidF_Kd

#define OPT_ERFTM_NOM  5000  //�������� ����������� �����������, ��
#define OPT_ERFDV_NOM   750  //������������ ���������� ��������, rpm

#define ECSIGN       0xBEDD  //��������� EEPROM

//----------------------------- ����������: ----------------------------------

static char Error;        //������� ��� ������
static char ErrorH;       //��� ������ �����������
static char DiagTimerH;   //������ ����������� �����������
static int  Temp[SPS];    //������ ���������� ��� �����������������
static char TempPnt;      //��������� ������� ����������

static char Tupd;         //���� ���������� �����������
static int  Tget;         //���������� �������� �����������, x0.1�C
static int  Tset;         //������������� �������� �����������, x0.1�C
static int  Pact;         //������� �������� �������� �����������, x0.1%
static int  Plim;         //����������� �������� �����������, x0.1%
static int  Pset;         //�������� �������� ����. (��� ������ ��� PID), x0.1%
static char PidH_Kp;      //Kp PIDH
static char PidH_Ki;      //Ki PIDH
static char PidH_Kd;      //Kd PIDH
static char PidH_En;      //���������� PID �����������
static char OptErHen;     //���������� ��������� ErH
static char OptErOen;     //���������� ��������� ErO
static char OptErSen;     //���������� ��������� ErS
static int  OptErHTm;     //�������� ��� ErH
static int  OptErHdT;     //������ ����������� ��� ErH
static int  OptErHvT;     //�������� ��������� ����������� ��� ErH

 //int  ECSign;    //��������� EEPROM
char EPidH_Kp;  //Kp PIDH � EEPROM
char EPidH_Ki;  //Ki PIDH � EEPROM
char EPidH_Kd;  //Kd PIDH � EEPROM
//int  EPlim;     //Plim � EEPROM
char EOptErHen; //���������� ErH � EEPROM
char EOptErOen; //���������� ErO � EEPROM
char EOptErSen; //���������� ErS � EEPROM
int  EOptErHTm; //�������� ��� ErH � EEPROM
int  EOptErHdT; //������ ����������� ��� ErH � EEPROM
int  EOptErHvT; //�������� ���. ����. ��� ErH � EEPROM


//-------------------------- ��������� �������: ------------------------------

int PidH(int ref, int inp, int out);   //PID-��������� �����������
#ifdef DFS
  int PidF(int ref, int inp, int out); //PID-��������� �����������
#endif

//------------------------ ������������� ������: -----------------------------

void Control_Init(void)
{
  //������ EEPROM:
    Control_SetPidH(PIDH_KP_NOM, PIDH_KI_NOM, PIDH_KD_NOM);
    Control_SetPlim(P_LIM_NOM);
    Control_SetErrH(1, 1, 1,
                    OPT_ERHTM_NOM, OPT_ERHDT_NOM, OPT_ERHVT_NOM);
  

  //������������� ����������:
  Error = ER_NO;

  Tset = 0;
  Pset = 0;
  Pact = 0;
  PidH_En = 1;
  TempPnt = 0;
  ErrorH = ER_NO;
}

//-------------------- ���������� ������ ����������: -------------------------

void Control_Exe(void)
{
    int dT = Tget - Temp[TempPnt];   //���������� ������������� ����������� 
    Temp[TempPnt++] = Tget;          //����. ����������� � ��������� ������
    if(TempPnt == SPS) TempPnt = 0;  //��������� ���������

    Pact = PidH(Tset, Tget, Pset);   //������ �������� �� PID-������
    if(Pact > Plim) Pact = Plim;     //����������� ������������ ��������:
    
    if (Pact < 0) 
      Pact = 0;
    
    ErrorH = ER_NO;                  //����� ���� ������
    if(DiagTimerH) DiagTimerH--;     //������ ����������� �����������
    else                             //���� ����� ���. ����������� �������,
    {
      if(OptErHen &&                 //���� ��������� ������ ErH ���������
         ((Tset - Tget) > OptErHdT) &&  //� ����������� ���� �������� �� DT
         (dT < OptErHvT))               //� �������� ����� ������ VT,
        ErrorH = ER_HEAT;               //�� ������ �����������
      if(OptErOen &&                 //���� ��������� ������ ErO ���������
         ((Tget - Tset) > OptErHdT) &&  //���� ����������� ���� �������� �� DT
         (dT > OptErHvT))               //� �������� ����� ������ VT,
        ErrorH = ER_OVER;               //�� ������ ���������
    }
    if(OptErSen &&                   //���� ��������� ������ ErS ���������
       (Tget > THJ_MAX))             //���� ����������� ���� THJ_MAX,
      ErrorH = ER_SENS;              //�� ������ �������
 

    Error = ErrorH;                  //�������������� ������ ��� DSS - ErrorH

}

//---------------------- PID-��������� �����������: --------------------------

//��������� ����������:
//Pid_Kp - ����������� ����������������
//Pid_Ki - ����������� ������������ 
//Pid_Kd - ����������� ����������������
//ref     - ������������� ��������
//inp (X) - ���������� ��������
//out (Y) - �����������

int PidH(int ref, int inp, int out)
{
  static int Xp = 0;  //���������� �������� inp
  static int Xpp = 0; //����-���������� �������� inp
  static long Y = 0;
  //���������� ������:
  int e = ref - inp;
  //���� ���������������� ������������:
  Y = Y - (long)PidH_Kp * (inp - Xp) * 10;
  //���� ������������ ������������:
  Y = Y + (long)PidH_Ki * e * TSAMPLE / 100;
  //���� ���������������� ������������:
  Y = Y - (long)PidH_Kd * (inp - 2 * Xp + Xpp) * 500 / TSAMPLE;
  //���������� ��������:
  Xpp = Xp; Xp = inp;
  if(!ref) Y = 0;
  if(!PidH_En) Y = (long)out << 8;
  //����������� ��������� ��������:
  if(Y > POW_MAX * 256L) Y = POW_MAX * 256L;
  if(Y < POW_MIN * 256L) 
    Y = POW_MIN * 256L;
  return(Y >> 8);
}

//----------------------- PID-��������� ����������: --------------------------

//��������� ����������:
//Pid_Kp - ����������� ����������������
//Pid_Ki - ����������� ������������ 
//Pid_Kd - ����������� ����������������
//ref     - ������������� ��������
//inp (X) - ���������� ��������
//out (Y) - �����������

#ifdef DFS
int PidF(int ref, int inp, int out)
{
  static int Xp = 0;  //���������� �������� inp
  static int Xpp = 0; //����-���������� �������� inp
  static long Y = 0;
  //��������� ������:
  int e = ref - inp;
  //���� ���������������� ������������:
  Y = Y - (long)PidF_Kp * (inp - Xp) / 10;
  //���� ������������ ������������:
  Y = Y + (long)PidF_Ki * e * VSAMPLE / 10000;
  //���� ���������������� ������������:
  Y = Y - (long)PidF_Kd * (inp - 2 * Xp + Xpp) / VSAMPLE;
  //���������� ��������:
  Xpp = Xp; Xp = inp;
  if(!ref) Y = 0;
  if(!PidF_En) Y = (long)out << 8;
  //����������� ��������� ��������:
  if(Y > PWM_MAX * 256L) Y = PWM_MAX * 256L;
  if(Y < PWM_MIN * 256L) Y = PWM_MIN * 256L;
  return(Y >> 8);
}
#endif

//----------------------------------------------------------------------------
//-------------------------- ������������ �������: ---------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//--------------------------------- �����: -----------------------------------
//----------------------------------------------------------------------------

//------------------------ ������ ����������� � ����: ------------------------

char Control_MainsOn(void)
{
  return(0);
}

//--------------------------- ������ ���� ������: ----------------------------

char Control_GetError(void)
{
  return(Error);
}

//----------------------------------------------------------------------------
//------------------------------ �����������: --------------------------------
//----------------------------------------------------------------------------

//------------------ ������ ����� ���������� �����������: --------------------

char Control_GetTupd(void)
{
  return(Tupd);
}

//---------------------- ������ ���������� �����������: ----------------------

int Control_GetTc(void)
{
  return(Tget);
}

//---------------------- ������� ���������� �����������: ----------------------

void Control_SetTc(int tTemp)
{
  Tget = tTemp ;
}

//------------------------- ��������� �����������: ---------------------------

void Control_SetT(int t)
{
  if(t < T_MIN) t = T_MIN;         //����������� �����������
  if(t > T_MAX) t = T_MAX;
  Tset = t;                        //��������� �����������
  DiagTimerH = OptErHTm / TSAMPLE; //������������ ������� �����������
}

//----------------------- ������ �������� �����������: -----------------------

int Control_GetT(void)
{
  return(Tset);
}

//--------------------------- ��������� ��������: ----------------------------

void Control_SetP(int p)
{
  if(p < POW_MIN) p = POW_MIN;
  if(p > POW_MAX) p = POW_MAX;
  Pset = p;
}

//------------------------ ������ ������� ��������: --------------------------

int Control_GetP(void)
{
  return(Pact);
}

//--------------------- ��������� ������������� PIDH: ------------------------

void Control_SetPidH(char kp, char ki, char kd)
{
  PidH_Kp = kp; if(EPidH_Kp != kp) EPidH_Kp = kp;
  PidH_Ki = ki; if(EPidH_Ki != ki) EPidH_Ki = ki;
  PidH_Kd = kd; if(EPidH_Kd != kd) EPidH_Kd = kd;
}

//---------------------- ������ ������������� PIDH: --------------------------

void Control_GetPidH(char *kp, char *ki, char *kd)
{
  *kp = PidH_Kp; *ki = PidH_Ki; *kd = PidH_Kd;
}

//--------------------------- ���������� PIDH: -------------------------------

void Control_SetEnH(char e)
{
  PidH_En = e;
}

//------------------------- ������ ���������� PIDH: --------------------------

char Control_GetEnH(void)
{
  return(PidH_En);
}

//-------------------- ��������� ���������� ������ ErH: ----------------------

void Control_SetErrH(char enh, char eno, char ens, int tm, int dt, int vt)
{
  if(tm < 0) tm = 0; if(tm > DELAY_MAX) tm = DELAY_MAX;
  if(dt < 0) dt = 0; if(dt > T_MAX) dt = T_MAX;
  if(vt < 0) vt = 0; if(vt > T_MAX) vt = T_MAX;
  OptErHen = enh; if(EOptErHen != enh) EOptErHen = enh;
  OptErOen = eno; if(EOptErOen != eno) EOptErOen = eno;
  OptErSen = ens; if(EOptErSen != ens) EOptErSen = ens;
  OptErHTm = tm; if(EOptErHTm != tm) EOptErHTm = tm;
  OptErHdT = dt; if(EOptErHdT != dt) EOptErHdT = dt;
  OptErHvT = vt; if(EOptErHvT != vt) EOptErHvT = vt;
}

//--------------------- ������ ���������� ������ ErH: ------------------------

void Control_GetErrH(char *enh, char *eno, char *ens, int *tm, int *dt, int *vt)
{
  *enh = OptErHen; *eno = OptErOen; *ens = OptErSen;
  *tm = OptErHTm; *dt = OptErHdT; *vt = OptErHvT;
}

//-------------------- ��������� ������������ ��������: ----------------------

void Control_SetPlim(int p)
{
  if(p < POW_MIN) p = POW_MIN;
  if(p > POW_MAX) p = POW_MAX;
  Plim = p;
  //if(EPlim != p) EPlim = p;
}

//---------------------- ������ ������������ ��������: -----------------------

int Control_GetPlim(void)
{
  return(Plim);
}

//--------------------- ��������� ����������� (dummy): -----------------------

void Control_SetComp(char c)
{
}

//---------------------- ������ ����������� (dummy): -------------------------

char Control_GetComp(void)
{
  return(0);
}

//----------------------------------------------------------------------------
//------------------------------ ����������: ---------------------------------
//----------------------------------------------------------------------------

#ifdef DFS

//-------------------- ������ ����� ���������� ��������: ---------------------

char Control_GetVupd(void)
{
  return(Vupd);
}

//----------------------- ������ ���������� ��������: ------------------------

int Control_GetVc(void)
{
  return(Vget);
}

//-------------------------- ��������� ��������: -----------------------------

void Control_SetV(int v)
{
  if(v < V_MIN) v = V_MIN;         //����������� ��������
  if(v > V_MAX) v = V_MAX;
  Vset = v;                        //��������� ��������
  DiagTimerF = OptErFTm / VSAMPLE; //�� ������������ ������� �����������
}

//------------------------ ������ �������� ��������: -------------------------

int Control_GetV(void)
{
  return(Vset);
}

//------------------- ��������� �������� PWM �����������: --------------------

void Control_SetW(char w)
{
  Wset = w;
}

//---------------- ������ �������� �������� PWM �����������: -----------------

char Control_GetW(void)
{
  return(Wact);
}

//--------------------- ��������� ������������� PIDF: ------------------------

void Control_SetPidF(char kp, char ki, char kd)
{
  PidF_Kp = kp; if(EPidF_Kp != kp) EPidF_Kp = kp;
  PidF_Ki = ki; if(EPidF_Ki != ki) EPidF_Ki = ki;
  PidF_Kd = kd; if(EPidF_Kd != kd) EPidF_Kd = kd;
}

//---------------------- ������ ������������� PIDF: --------------------------

void Control_GetPidF(char *kp, char *ki, char *kd)
{
  *kp = PidF_Kp; *ki = PidF_Ki; *kd = PidF_Kd;
}

//--------------------------- ���������� PIDF: -------------------------------

void Control_SetEnF(char e)
{
  PidF_En = e;
}

//------------------------- ������ ���������� PIDF: --------------------------

char Control_GetEnF(void)
{
  return(PidF_En);
}

//-------------------- ��������� ���������� ������ ErF: ----------------------

void Control_SetErrF(char en, int tm, int dv)
{
  if(tm < 0) tm = 0; if(tm > DELAY_MAX) tm = DELAY_MAX;
  if(dv < 0) dv = 0; if(dv > V_MAX) dv = V_MAX;
  OptErFen = en; if(EOptErFen != en) EOptErFen = en;
  OptErFTm = tm; if(EOptErFTm != tm) EOptErFTm = tm;
  OptErFdV = dv; if(EOptErFdV != dv) EOptErFdV = dv;
}

//--------------------- ������ ���������� ������ ErF: ------------------------

void Control_GetErrF(char *en, int *tm, int *dv)
{
  *en = OptErFen; *tm = OptErFTm; *dv = OptErFdV;
}

#endif

//----------------------------------------------------------------------------
