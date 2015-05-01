//----------------------------------------------------------------------------

//DSS-90/DFS-90
//������ ���������� �������� � ������� �������: ������������ ����

//----------------------------------------------------------------------------

#ifndef ControlH
#define ControlH

//------------------------------ ���������: ----------------------------------
#define TSAMPLE     50
#define THJ_MAX     450
#define T_MIN         0  //����������� �����������, x0.1�C
#define T_MAX   THJ_MAX  //������������ �����������, x0.1�C
#define V_MIN         0  //����������� ��������, rpm
#define V_MAX      9999  //������������ ��������, rpm
#define COMP_MIN      0  //�����������  �����������
#define COMP_MAX      9  //������������ �����������
#define DELAY_MAX 30000  //������������ �������� ��������, ��

#define POW_MIN 0
#define POW_MAX 48
//���� ������:
enum
{
  ER_NO,   //��� ������
  ER_FAN,  //������ �����������
  ER_HEAT, //������ �����������
  ER_OVER, //������ ���������
  ER_SENS  //������ �������
};

//-------------------------- ��������� �������: ------------------------------

void Control_Init(void);      //������������� ������
void Control_Exe(void);     //���������� ��������

char Control_MainsOn(void);   //������ ����������� � ����
char Control_GetError(void);  //������ ���� ������

char Control_GetTupd(void);   //������ ����� ���������� �����������
int  Control_GetTc(void);     //������ ������� �����������, x0.1�C
void Control_SetT(int t);     //��������� �����������, x0.1�C
int  Control_GetT(void);      //������ ������������� �����������, x0.1�C
void Control_SetP(int p);     //��������� ��������, x0.1%
int  Control_GetP(void);      //������ ������� ��������, x0.1%
void Control_SetPidH(char kp, char ki, char kd); //���. �����. PIDH
void Control_GetPidH(char *kp, char *ki, char *kd); //������ �����. PIDH
void Control_SetEnH(char e);  //���������� PIDH
char Control_GetEnH(void);    //������ ���������� PIDH
void Control_SetErrH(char enh, char eno, char ens,
                     int tm, int dt, int vt); //��������� ���������� ErH
void Control_GetErrH(char *enh, char *eno, char *ens,
                     int *tm, int *dt, int *vt); //������ ���������� ErH
void Control_SetPlim(int p);  //��������� ������������ ��������, x0.1%
int  Control_GetPlim(void);   //������ ������������ ��������, x0.1%
void Control_SetComp(char c); //��������� �����������
char Control_GetComp(void);   //������ �����������
void Control_SetTc(int tTemp);
//----------------------------------------------------------------------------

#endif
