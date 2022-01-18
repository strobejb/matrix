#ifndef MATRIX_INC
#define MATRIX_INC

int jjrand(void);
extern int maxcols, maxrows;
extern int numrows, numcols;
extern int xChar, yChar;

#define DENSITY_MIN 5
#define DENSITY_MAX 50

#define SPEED_MIN	1
#define SPEED_MAX	10

#define MSGSPEED_MAX 500
#define MSGSPEED_MIN 50

#define FONT_MIN	8
#define FONT_MAX	30

struct Matrix
{
	int state;			//0 (insert blanks) or 1 (insert digits)
	int statecount;		//how long to stay in current state, counts down

	int  *run;			//a vertical run of digits
	int  runlen;		//length of the run
	bool *update;		//boolean array which identifies which digits need to be redrawn

	int initcount;		//counter before we are allowed to start scrolling
	bool started;		//have we started this run yet??

	int blippos;		//vertical position of the bright "blip" that shoots downwards
	int bliplen;		//how long (a random value) does the blip last?

	void Init(int runlength)
	{
		runlen = runlength + 1;				//1 for luck
		run    = new int[runlen];
		update = new bool[runlen + 30];		//space for overflow by blips

		state = jjrand() & 1;
		statecount = jjrand() % 20 + 3;

		initcount = jjrand() % maxcols;		//count before we are allowed to start
		started = false;

		for(int i = 0; i < runlen; i++)
		{
			run[i] = -1;
			update[i] = false;
		}
		
		blippos = 0;	
		bliplen = jjrand() % 50 + numrows;
	}

	Matrix()//int runlength)
	{
	}

	~Matrix() { delete[] run; delete[] update; }
	void ScrollDown(HDC hdc);
	void jjrandomise();

};




#endif