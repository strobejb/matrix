#ifndef _MSGINC
#define _MSGINC

#define MAXMESSAGES 16
#define MAXMSGLEN 64
#define MSGWIDTH  256
#define MSGHEIGHT 256

extern int nNumMessages;
extern char szMessages[][MAXMSGLEN];

//
//	A class which handles matrix messages appearing
//
class Message
{
public:
	char curmsg[MAXMSGLEN];

	bool bitmap[MSGWIDTH][MSGHEIGHT];
	bool visible[MSGWIDTH][MSGHEIGHT];

	bool state;

	Message();


	int rand();//unsigned short reg) 

	void SetMessage(char *newmsg, int fontsize);
	void Reveal(int amt);
	void ShowMessage(HDC hdc);
	void HideMessage(void);
	void ClearMessage(void);
	void Preview(HDC hdc);
};

void InitMessage(void);
void DeInitMessage(void);
void DoMessages(HDC hdc);

#endif