/**************************
 * Includes
 *
 **************************/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <work.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>

const double PI = 3.14159265359f; 





/**************************
 * Function Declarations
 *
 **************************/

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);
int InitGL();
void ReSizeGLScene(GLsizei width, GLsizei height);
GLuint loadBMP_custom(const char * imagepath);
void ProcessColorBuffer ( int x, int y );



int SimpleCube(float x, float y, float z, float baseU, float baseV);
int SimpleStair(float x, float y, float z, float baseU, float baseV);
int SimplePlane(float x, float y, float z, float baseU, float baseV);
int SimpleCubePick(float x, float y, float z, float r, float g, float b);

void DrawInterface(GLuint texture, int sizeX, int sizeY);
void DrawInterfaceCell(int i, float baseU, float baseV);
void DrawHealthBar(GLuint texture,int Health,int sizeX,int sizeY);
void DrawInventoryBox(GLuint texture, int sizeX,int sizeY);
void DrawInventory(GLuint texture, int sizeX, int sizeY);
void DrawInventoryCell(int i, int j, float baseU, float baseV);
void DrawItem(int x, int y, float baseU, float baseV);


void BlockCreatedInsideChunk(int x, int y, int z);
void InitInventoryTest();
void StartInterfaceDraw(int sizeX, int sizeY);
void EndInterfaceDraw();

int SaveChunk(int target);
/*******************
* Blocks
*
*******************/

int StandartBlock(float x, float y, float z, float baseU, float baseV);
int Layer3Block(float x, float y, float z, float baseU1, float baseV1, 
float baseU2, float baseV2, float baseU3, float baseV3);

int Layer3FrontBlock(float x, float y, float z, float baseU1, float baseV1, 
float baseU2, float baseV2, float baseU3, float baseV3);

bool	keys[256] = {false};			// Array Used For The Keyboard Routine

HDC hDC;


#define COLOR_BUFFER_SIZE   320000

typedef unsigned __int8 vector3ub[3]; 
vector3ub Select;
struct cbInfo_t
{
  int Name;
  vector3ub  Color;
};



int CurrentObject;

/**************************
 * ItemCloass
 *
 *************************/

class Item
{	
	public:
		int Id, Count;
		Item (int nId, int nCount)
		{
			Id = nId;
			Count = nCount;
		};
		virtual void UseItem()
		{
			
		};
};

class ItemResource : public Item
{
	public:
		ItemResource(int nId, int nCount):Item(nId, nCount)
		{
		};
		virtual void UseItem() override 
		{
			
		};
};

class ItemTool : public Item
{
	int Capacity, Power;
	public:
		ItemTool(int nId, int nCapacity, int nPower):Item(nId, 1)
		{
			Capacity = nCapacity;
			Power = nPower;
		};
		virtual void UseItem() override 
		{
			
		};
};

Item* Inventory[32]; // Data for Inventory
void DrawItemOnCursor(GLuint texture, int sizeX, int sizeY, int x, int y, Item* TakenItem);



/**************************
 * Game Data
 *
 *************************/

int ChunkChange(int x, int y);


int LoadChunk(int x, int y)
{
	FILE* fp;
	char str[255] = "map/m(";
	char str1[32];
    sprintf(str1, "%d", x);
    strcat(str,str1);
    strcat(str,",");
    sprintf(str1, "%d", y);
    strcat(str,str1);
    strcat(str,").map");
	if((fp=fopen(str, "rb"))==NULL)
		if ((fp=fopen("map/test.map", "rb"))==NULL) /* Insert Generator here*/
			exit(1);
	ChunkTop++;
	if (ChunkTop == maxChunk)	
	{
		ChunkTop = 0;
		while (data[ChunkTop].active == 1)
			ChunkTop++;
	}
	data[ChunkTop].x = x;
	data[ChunkTop].y = y;
	data[ChunkTop].active = 1;
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			for (int k = 0; k < 64; k++)
			{
				fread(&data[ChunkTop].map[i][j][k], sizeof(int), 1, fp);
				data[ChunkTop].status[i][j][k] = 0;
			}
	fclose(fp);			
	return ChunkTop;
}

int InitWorld()
{
	int wcount = 0;
	for (int z=-1; z<=1; z++)
	 for (int x=-1; x<=1; x++)
	  {
	  	w[wcount] = LoadChunk(x,z);
	  	wcount++;
	  }
	tChunk None;
	None.status[0][0][0] = -1;
	for (int i=0; i<9; i++)
	{
		tChunk ct1, ct2, ct3, ct4;
		int t1, t2, t3, t4;
		GetNearestChunks(i,t4,t3,t2,t1,3);
		if (t1 == -1)
			ct1 = None;
		else ct1 = data[t1];	
		if (t2 == -1)	
			ct2 = None;
		else ct2 = data[t2];
		if (t3 == -1)
		    ct3 = None;
		else ct3 = data[t3];    
		if (t4 == -1)
			ct4 = None;
		else ct4 = data[t4];			
		OptimizeChunk(data[w[i]], ct1, ct2, ct3, ct4);  
	}	
	return wcount;  
}

int LoadWorld(int x, int y)
{
	FILE* fp;
	char sx[6];
	char sy[6];
	itoa (x,sx,10);
	itoa (y,sy,10);
	if((fp=fopen("map/m(0,0).map", "rb"))==NULL)
		exit(1);
	
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			for (int k = 0; k < 64; k++)
				fread(&world.map[i][j][k], sizeof(int), 1, fp);
	fclose(fp);
}



void DrawWorld(int ID, int x, int y)
{
	
	x = x*32;
	y = y*32;
	
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			for (int k = 0; k < 64; k++)
			{
				if (data[ID].status[i][j][k] != NoDraw)
				{
					switch(data[ID].map[i][j][k])
					{
						case 0: 	// Air
							break;  
						case 1:		// GrassBlock
						    Layer3Block(2*i+x,2*k,2*j+y,2,1,1,1,0,1); 
							break;
						case 2: 	// Wood
							Layer3Block(2*i+x,2*k,2*j+y,4,1,3,1,4,1);
							break;
						case 3: 	// Stone
						    StandartBlock(2*i+x,2*k,2*j+y,0,0);
						    break;
						case 4: 	// Dirt
						    StandartBlock(2*i+x,2*k,2*j+y,0,1);
							break;
						case 5: 	// Cobblestone
							StandartBlock(2*i+x,2*k,2*j+y,1,0);
							break;
						case 6: 	// Cobblestone with leaves
							StandartBlock(2*i+x,2*k,2*j+y,0,2);
							break;
						case 7: 	// Coal Ore
							StandartBlock(2*i+x,2*k,2*j+y,4,0);
							break;
						case 8: 	// Iron Ore
							StandartBlock(2*i+x,2*k,2*j+y,5,0);
							break;
						case 9: 	// Gold Ore
							StandartBlock(2*i+x,2*k,2*j+y,6,0);
							break;
						case 10: 	// Diamond Ore
							StandartBlock(2*i+x,2*k,2*j+y,7,0);
							break;
						case 300:	// Vitalium Ore
							StandartBlock(2*i+x,2*k,2*j+y,8,0);
							break;	
						case 301: // Copper Ore
							StandartBlock(2*i+x,2*k,2*j+y,9,0);
							break;
						case 302: // Tin Ore
							StandartBlock(2*i+x,2*k,2*j+y,10,0);
							break;
						case 303: // Alluminium Ore
							StandartBlock(2*i+x,2*k,2*j+y,11,0);
							break;
						case 304: // Uranium Ore
							StandartBlock(2*i+x,2*k,2*j+y,12,0);
							break;
						case 305: // Titan Ore
							StandartBlock(2*i+x,2*k,2*j+y,13,0);
							break;
						case 306: // Silver Ore
							StandartBlock(2*i+x,2*k,2*j+y,14,0);
							break;
						case 307: // Sapphire Ore
							StandartBlock(2*i+x,2*k,2*j+y,15,0);
							break;
						case 308: // Ruby Ore
							StandartBlock(2*i+x,2*k,2*j+y,16,0);
							break;
						case 309: // Emerald Ore
							StandartBlock(2*i+x,2*k,2*j+y,17,0);
							break;
						case 420:	// Wood
							Layer3Block(2*i+x,2*k,2*j+y,4,1,3,1,4,1);
							break;
						case 421:	// Wood sheets
							StandartBlock(2*i+x,2*k,2*j+y,0,3);
							break;
						case 422:	// Pin tree
							Layer3Block(2*i+x,2*k,2*j+y,6,1,5,1,6,1);
							break;
						case 1000: // Stone furnace
							Layer3FrontBlock(2*i+x,2*k,2*j+y,0,4,1,4,2,4);
							break; 	
					}
				}
			}
}

void InitDrawWorld(int wcount)
{
	for (int i=0; i<wcount; i++)
	   DrawWorld(w[i], data[w[i]].x, data[w[i]].y);
}

/**************************
 * WinMain
 *
 **************************/

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int iCmdShow)
{
	
    WNDCLASS wc;
    HWND hWnd;

    HGLRC hRC;        
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;
    DWORD sTime = 0;
    DWORD sTimeOld = 0;
    float deltaTime = 0.0f; 
    float camx = 0;
	float camy = 0;
	float camz = 0;
	float objx = 10;
	float objy = 67.75f;
	float objz = 10;
	float mx = 0;
	float my = 0;
	float mz = 0;
	
	int wdSizeX = 1024;
	int wdSizeY = 768;
	int wdStartX = 400;
	int wdStartY = 220;
	float radius = 100;
	float t = 0.5f; // vertical
	float ph = 0.5f; // horizontal
	float speed = 3.0f;
	float borderX = 31;
	float borderZ = 31;
	
	bool OnGround = true;
	float sy = 0.05f;
	float sumsy = -0.3f;
	
	float HPregen = 0;
	int HealthPoints = 100;
	
	int interfaceID[8] = {0};
		
	bool InventoryOpen = false;
	bool TakeItem = false;
	int IdTakenItem = 0;
	Item* TakenItem;
	int OldIdCell;
	
	float BlockTakeItem = 0.0f;
	float inventoryblock = 0.0f;
	float BlockDeleteOb = 0.0f;
	
	int StrokeLong = 3;
	int Central = GetCentralChunk(StrokeLong);
	
    /* register window class */
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "GLSample";
    RegisterClass (&wc);

    /* create main window */
    hWnd = CreateWindow (
      "GLSample", "NexusCraft", 
      WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
      wdStartX, wdStartY, wdSizeX, wdSizeY,
      NULL, NULL, hInstance, NULL);

    /* enable OpenGL for the window */
    EnableOpenGL (hWnd, &hDC, &hRC);
    InitGL();
    ReSizeGLScene(wdSizeX,wdSizeY);
    sTime = GetTickCount(); 
    ShowCursor(FALSE);
    
    /* init some structures */
    glEnable(GL_TEXTURE_2D);

   // glDisable(GL_COLOR);
    glClearColor (0.79f, 0.79f, 0.95f, 0.0f);
    GLuint blocktex = loadBMP_custom("textures/texturemap.bmp");    
	GLuint interfacetex = loadBMP_custom("textures/Interfacemap.bmp");   
	GLuint HPBAR = loadBMP_custom("textures/HPbar.bmp");   
	GLuint inventorybox = loadBMP_custom("textures/inventorybox.bmp");
	/* TGA test */
	Tga info = Tga("textures/book.tga");
    GLuint testTga = 0;
    glGenTextures(1, &testTga);
    glBindTexture(GL_TEXTURE_2D, testTga);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, info.GetWidth(), info.GetWidth(), 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, info.GetPixels().data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D,blocktex);
	/* Set Drawing Type for Polygons */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    InitWorld();

    /* Some values to initialize */
	mx = 0;
	my = 0;
	InitInventoryTest();

	CheckStartPosition(objy, objx, objz, data[Central]);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
        else
        {
        	
        	/* Get DeltaTime*/
        	sTimeOld = sTime;
        	sTime = GetTickCount();
        	deltaTime = (sTime - sTimeOld);
        	
            /* OpenGL animation code goes here */

 			glBindTexture(GL_TEXTURE_2D,blocktex);

            glPushMatrix ();  /* Start Drawing */
            
            gluLookAt(camx,camy,camz,objx-mx,objy-my,objz-mz-0.01f,0.0f,1.0f,0.0f); // Set Camera
            

		//	if (keys[0x5A])
        //	{
        	glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
        	ProcessColorBuffer((0.5f*wdSizeX),(0.5f*wdSizeY));
		//	}
			glClearColor (0.79f, 0.79f, 0.95f, 0.0f);
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// Execute world draw
            InitDrawWorld(9); 
            if (Select[0] < 255 && Select[1] < 255 && Select[2] < 255)
				SimpleCubePick(Select[0]*2+targetx*32, Select[1]*2,Select[2]*2+targety*32, 0.2f, 0.0f , 1.0f);
            glPopMatrix ();   /* End Drawing */
            
            /* Interface draw*/
            glEnable(GL_ALPHA_TEST);
    		glEnable(GL_BLEND);
    		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            StartInterfaceDraw(wdSizeX, wdSizeY);
            glColor3f(1.0f, 1.0f, 1.0f);
            //DrawInterface(interfacetex, wdSizeX, wdSizeY);
            DrawInterface(testTga, wdSizeX, wdSizeY);
            

            DrawItem(wdSizeX*0.5f-4, wdSizeY*0.5f-4, 0, 1);

            
            DrawHealthBar(HPBAR, HealthPoints,wdSizeX,wdSizeY);
            tagPOINT Mouse;
        	GetCursorPos(&Mouse);
            if (InventoryOpen)
			{
				
            	DrawInventoryBox(inventorybox, wdSizeX, wdSizeY);
            //	DrawInventory(interfacetex, wdSizeX, wdSizeY);
            	DrawInventory(testTga, wdSizeX, wdSizeY);
            	if (TakeItem)
            		DrawItemOnCursor(testTga, wdSizeX, wdSizeY, Mouse.x - wdStartX - 32, Mouse.y - wdStartY - 32, TakenItem);
        	}
            EndInterfaceDraw();        	
        	glDepthMask(GL_TRUE);
        	glDisable(GL_ALPHA_TEST);
    		glDisable(GL_BLEND);
    		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            SwapBuffers (hDC);
            
            

        	
        	
			HWND Active = GetActiveWindow();
			if (Active == hWnd)
			{
	        	if (!InventoryOpen)
	        	{
	        		while(ShowCursor(FALSE) >=0);
	        		
	        		SetCursorPos(wdStartX+(0.5*wdSizeX),wdStartY+(0.5*wdSizeY));
	        		
	        		t -= (wdStartX+(0.5*wdSizeX)-Mouse.x) * 0.0013f;
					ph -= (Mouse.y - (wdStartY+(0.5*wdSizeY))) * 0.0013f;
				}
	        	else 
				{
					while(ShowCursor(TRUE) <=0);
				}
			}
			else ShowCursor(TRUE);
        	// Move Camera
        	

		//	t -= (wdStartX+(0.5*wdSizeX)-Mouse.x) * 0.0013f;
		//	ph -= (Mouse.y - (wdStartY+(0.5*wdSizeY))) * 0.0013f;
			
			if (ph < 0.035f)
            ph = 0.035f;
            
            if (ph > (PI- 0.035f))
            ph = (PI) - 0.035f;
            
            float objOldx, objOldy, objOldz;
            
            objOldx = objx; /* Save old coordinates */
            objOldy = objy;
            objOldz = objz;
            
             
            
        	SphereToCart (radius, ph ,t , camx, camy, camz);
        	
        	float speedx, speedz, len;
        	len = sqrt (camx*camx + camz*camz);
        	speedx = camx / len;
        	speedz = camz / len;
        	
	       	if (GetAsyncKeyState(VK_LBUTTON) && BlockTakeItem > 1.0f && InventoryOpen && (Mouse.x >= 260 + 400 && Mouse.x <= 772 + 400 && Mouse.y >= 310 + 220 && Mouse.y <= 630 + 220))
        	{
        		BlockTakeItem = 0.0f;
				if (TakeItem)
        		{
   					int a = 24;
					int x = (Mouse.x - 400 - 260)/64;
					int y = ((Mouse.y - 220 - 310)/64)-1;
					x = a+x;
					int Id = x;
					for (int i = y; i > 0; i--)
						Id -= 8;
							
					if (Inventory[Id] == NULL || Inventory[Id]->Id == 0)
					{
						Inventory[Id] = TakenItem;
						TakeItem = false;
					}
					else if (Inventory[Id] != 0 && Inventory[Id]->Id != 0)
					{
						Inventory[OldIdCell] = Inventory[Id];
						Inventory[Id] = TakenItem;
						TakeItem = false;
					}
								
				}
				else
				{			
					int a = 24;
					int x = ((Mouse.x - 400 - 260)/64);
					int y = ((Mouse.y - 220 - 310)/64)-1;
					x = a+x;
					int Id = x;
					for (int i = y; i > 0; i--)
						Id -= 8;
								
					if (Inventory[Id] != 0)
					{
						IdTakenItem = Inventory[Id]->Id;
						TakenItem = Inventory[Id];
						Inventory[Id] = NULL;
					}
					else Id = 0;
							 
					if (IdTakenItem != 0)
					{
						OldIdCell = Id;
						TakeItem = true;
					}
				}
			}
			
			if (GetAsyncKeyState(VK_RBUTTON) && TakeItem == true && BlockTakeItem >= 1.0f && InventoryOpen && (Mouse.x >= 260 + 400 && Mouse.x <= 772 + 400 && Mouse.y >= 310 + 220 && Mouse.y <= 630 + 220))
    		{
    			TakeItem = false;
    			Inventory[OldIdCell] = TakenItem;
			}
			
        	if (keys[0x49] && inventoryblock >= 1.0f)
        	{
        		inventoryblock = 0.0f;
        		InventoryOpen = !InventoryOpen;	
        		SetCursorPos(wdStartX+(0.5*wdSizeX),wdStartY+(0.5*wdSizeY));
			}
			
			if (inventoryblock < 1.0f)
				inventoryblock += 0.1f;
        	if (BlockTakeItem < 1.0f)
        		BlockTakeItem += 0.1f;
        	if (BlockDeleteOb < 1.0f)
        		BlockDeleteOb += 0.1f;
        	
        	if (keys[0x57]) // W
        	{
        		objx -= speedx * speed * deltaTime * 0.002f;
				objz -= speedz * speed * deltaTime * 0.002f;	
    		}
        	if (keys[0x41]) // A
        	{
        		objx -= speedz * speed * deltaTime * 0.002f;
				objz += speedx * speed * deltaTime * 0.002f;
			}
        	if (keys[0x53]) // S
        	{
        		objx += speedx * speed * deltaTime * 0.002f;
				objz += speedz * speed * deltaTime * 0.002f;	
			}
        	if (keys[0x44]) // D
        	{
        		objx += speedz * speed * deltaTime * 0.002f;
				objz -= speedx * speed * deltaTime * 0.002f;
			}
			

			
			if (keys[VK_SPACE])
        	{
        		OnGround = false;
			}
        	
        //	CheckBorder(borderX, borderZ, objx, objz);
        	
        	CollisionCheckMulti( objOldx , objOldy , objOldz , objx , objy , objz );
        	CollisionCheckDiagonal( objOldx , objOldy , objOldz , objx , objy , objz );
        	
        	int dx, dy;
        	
        	CheckChunk(objx, objy, objz, dx, dy);
        	

        	if (dx != 0 || dy != 0) /* Chunk update (move)*/
        	{
		   		ChunkChange(dx, dy);
		   		targetx += dx;
        		targety += dy;
        	}

			
			if (keys[0x45])
			{
				float 	fx = ((objx+1)/2);
				float	fy = ((objy-2.75f)/2);
				float	fz = ((objz+1)/2);
				int x = trunc(fx)-data[w[Central]].x*16;
				int y = trunc(fy);
				int z = trunc(fz)-data[w[Central]].y*16;
				if (fx < 0)
					x--;
				if (fz < 0)
					z--;
				
        		float fy2 = ((objy + 1.25f)/2);
        		int y2 = trunc(fy2);
				if (data[w[Central]].map[x][z][y2] != 0 && data[w[Central]].map[x][z][y] == 0)
        		{
        			
        		}
        		else
				{
					int Left,Right,Top,Bottom;
					
					GetNearestChunks(Central, Top, Left, Bottom,Right,StrokeLong);
					
					objy += 1.0f;
					float fy3 = ((objy - 2.75f)/2);
        			int y3 = trunc(fy3);
        			data[w[Central]].map[x][z][y3] = 4;
        			if (x != 0 && x != 15 && z != 0 && z != 15 && y != 0 && y != 63)
        				BlockCreatedInsideChunk(data[w[Central]],x,y,z);
        				
        			if (x == 0 && z != 0 && z != 15)
						BlockCreatedBorderChunk(data[w[Central]],data[w[Right]],x,y,z);
					if (x == 15 && z != 0 && z != 15)
						BlockCreatedBorderChunk(data[w[Central]],data[w[Left]],x,y,z);
					if (z == 0 && x != 0 && x != 15)
						BlockCreatedBorderChunk(data[w[Central]],data[w[Bottom]],x,y,z);
					if (z == 15 && x != 0 && x != 15)
						BlockCreatedBorderChunk(data[w[Central]],data[w[Top]],x,y,z);
				}
			}
			if (GetAsyncKeyState(VK_LBUTTON) && !InventoryOpen && BlockDeleteOb >= 1.0f)
        	{
        		int Left,Right,Top,Bottom;

        		BlockDeleteOb = 0.0f;
        		data[w[Central]].map[Select[0]][Select[2]][Select[1]] = 0;
        		
        		GetNearestChunks(Central,Top,Left,Bottom,Right,StrokeLong);
        		
        		if (Select[0] != 0 && Select[0] != 15 && Select[2] != 0 && Select[2] != 15 && Select[1] != 0 && Select[1] != 63)
					BlockDeletedInsideChunk(data[w[Central]],Select[0],Select[1],Select[2]);
			
				if (Select[0] == 0 && Select[2] != 0 && Select[2] != 15)
					BlockDeletedBorderChunk(data[w[Central]],data[w[Right]],Select[0],Select[1],Select[2]);
				if (Select[0] == 15 && Select[2] != 0 && Select[2] != 15)
					BlockDeletedBorderChunk(data[w[Central]],data[w[Left]],Select[0],Select[1],Select[2]);
				if (Select[2] == 0 && Select[0] != 0 && Select[0] != 15)
					BlockDeletedBorderChunk(data[w[Central]],data[w[Bottom]],Select[0],Select[1],Select[2]);
				if (Select[2] == 15 && Select[0] != 0 && Select[0] != 15)
					BlockDeletedBorderChunk(data[w[Central]],data[w[Top]],Select[0],Select[1],Select[2]);
					
				if (Select[0] == 0 && Select[2] == 0)
					BlockDeletedAngleChunk(data[w[Central]],data[w[Right]],data[w[Bottom]],Select[0],Select[1],Select[2]);
				if (Select[0] == 15 && Select[2] == 0)
					BlockDeletedAngleChunk(data[w[Central]],data[w[Bottom]],data[w[Left]],Select[0],Select[1],Select[2]);
				if (Select[0] == 0 && Select[2] == 15)
					BlockDeletedAngleChunk(data[w[Central]],data[w[Top]],data[w[Right]],Select[0],Select[1],Select[2]);
				if (Select[0] == 15 && Select[2] == 15)
					BlockDeletedAngleChunk(data[w[Central]],data[w[Left]],data[w[Top]],Select[0],Select[1],Select[2]);
        		
        	}
			
			if (OnGround == false)
			{
				float 	fx = ((objx+1)/2);
				float	fy = ((objy-1.75f)/2);
				float	fz = ((objz+1)/2);
				int x = trunc(fx)-data[w[Central]].x*16;
				int y = trunc(fy);
				int z = trunc(fz)-data[w[Central]].y*16;
				if (fx < 0)
					x--;
				if (fz < 0)
					z--;
				
				
        		float fy2 = ((objy + 1.25f)/2);
        		int y2 = trunc(fy2);
        		if (data[w[Central]].map[x][z][y2] != 0)
        		{
        			sumsy = 0.0f;
				}
        		
        		if (data[w[Central]].map[x][z][y] != 0)
        		{
        			if (sumsy > 0.7f)
        				HealthPoints = HealthPoints - sumsy*85.0f;
        			OnGround = true;	
        			sumsy = -0.3f;
				}
				else
				{
					sumsy += sy*sy*5*deltaTime*0.05f;
				}
				
				objy -= sumsy*deltaTime*0.03f;
			}	
			
			float 	fx = ((objx+1)/2);
			float	fy = ((objy-2.75f)/2);
			float	fz = ((objz+1)/2);
			int x = trunc(fx)-data[w[Central]].x*16;
			int y = trunc(fy);
			int z = trunc(fz)-data[w[Central]].y*16;
			if (fx < 0)
				x--;
			if (fz < 0)
				z--;
			
			
        	if (data[w[Central]].map[x][z][y] == 0 && OnGround)
        	{
        		OnGround = false;
        		sumsy = 0.0f;
			}
			
        	mx = camx;
        	my = camy;
        	mz = camz;

        	camx = objx + camx*0.01f;
        	camy = objy + camy*0.01f;
        	camz = objz + camz*0.01f;
        	
        	CollisionCheckMulti(objx, objy-1.0f, objz, camx, camy, camz); 
        	

			
			if (HealthPoints < 100)
			{
				HPregen += 0.05f * deltaTime;
				if (HPregen >= 50)
				{
					HPregen = HPregen - 50;
					HealthPoints += 5;
				}
			}
			
			if (HealthPoints < 0)
				HealthPoints = 0;
        	
        	// Set Title for Window
        	char title[64] = "NexusCraft v. 0.2.4  [ X = ";
    		char temp[20];
    		sprintf(temp, "%f", objx);
    		strcat(title, temp);
    		strcat(title, "; Z = ");
    		sprintf(temp, "%f", objz);
    		strcat(title, temp);
    		strcat(title, "; Y =");
    		sprintf(temp, "%f", objy);
    		strcat(title, temp);
    		strcat(title, " ] Picket ID -> ");
    		sprintf(temp, "%d", CurrentObject);
    		strcat(title,temp);
    		strcat(title, " (X=");
    		sprintf(temp, "%d", Select[0]);
    		strcat(title, temp);
    		strcat(title, "; Y=");
    		sprintf(temp, "%d", Select[1]);
    		strcat(title, temp);
    		strcat(title, "; Z=");
    		sprintf(temp, "%d", Select[2]);
    		strcat(title, temp);
    		strcat(title, ")");
			
    		SetWindowTextA(hWnd, title);
		}
    }
    
	ShowCursor(TRUE); // Return Cursor
	
	for (int i=0; i<9; i++)
		SaveChunk(i);
	
    /* shutdown OpenGL */
    DisableOpenGL (hWnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow (hWnd);

    return msg.wParam;
    
    
}


/********************
 * Window Procedure
 *
 ********************/

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage (0);
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;
        default:
			{
				keys[wParam] = TRUE;					// If So, Mark It As TRUE
				return 0;								// Jump Back
			}   
        }
        return 0;
    case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
    default:
        return DefWindowProc (hWnd, message, wParam, lParam);
    }
}


/*******************
 * Enable OpenGL
 *
 *******************/

void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC (hWnd);

    /* set the pixel format for the DC */
    ZeroMemory (&pfd, sizeof (pfd));
    pfd.nSize = sizeof (pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | 
      PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat (*hDC, &pfd);
    SetPixelFormat (*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );

}


/******************
 * Disable OpenGL
 *
 ******************/

void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
}

/******************
* Init Opengl
*
******************/

int InitGL()										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	return TRUE;										// Initialization Went OK
}

void ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

/******************
* Geometric figures
*
******************/

int SimpleCube(float x, float y, float z, float baseU, float baseV)
{
	glColor3f(1.0f,1.0f,1.0f);
	baseU = baseU / 64.0f;
	baseV = (64.0f - baseV - 1.0f) / 64.0f;
	glBegin (GL_QUADS);
	
	
	//Front side
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x-1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU + 0.0f,baseV);              		 glVertex3f (x-1.0f, y-1.0f, z+1.0f);	
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 		     glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Top side
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex3f (x+1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU + 0.0f,baseV); 	 				 glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	
	//Bottom side
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f);		 glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU + 0.0f,baseV);			  		 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	
	//Left side
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 	     	 glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	//Right side
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 		 	 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Back side
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU + 0.0f,baseV); 		   	 		 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	glEnd ();
	return 0;
}

int SimpleCubePick(float x, float y, float z, float r, float g, float b)
{
	glColor3f(r,g,b); 
	glBegin (GL_QUADS);
	
	
	//Front side
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z+1.0f);	
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z+1.0f);	
	glColor3f(r,g,b);   glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Top side
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z+1.0f);	
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	
	//Bottom side
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	
	//Left side
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	//Right side
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Back side
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glColor3f(r,g,b); 	glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	glEnd ();
	return 0;
}

int SimplePlane(float x, float y, float z, float baseU, float baseV) /* Plane - Simple Plane */
{
	baseU = baseU / 64.0f;
	baseV = (64.0f - baseV - 1.0f) / 64.0f;
	glBegin(GL_QUADS);
	
	//Front side	/*0.5f,0.5f*/
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z+1.0f);	// TexCoord 1 1
	glTexCoord2f(baseU + 1.0f/64.0f,baseV);              glVertex3f (x-1.0f, y+1.0f, z+1.0f);	// TexCoord 1 0
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex3f (x-1.0f, y-1.0f, z+1.0f);	// TexCoord 0 0
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex3f (x+1.0f, y-1.0f, z+1.0f);	// TexCoord 0 1
	
	glEnd();
}

int SimpleStair(float x, float y, float z, float baseU, float baseV)
{
	glBegin(GL_QUADS);
	
	//Back side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+1.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z+1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z+1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+1.0f,z+1.0f);
	
	//Left side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+1.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x-1.0f,y+1.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y+0.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z+1.0f);
	
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f, y-1.0f, z+1.0f);
	
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z+0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z-1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z-1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y-1.0f,z+0.0f);
	
	//Right side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+1.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y+1.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x+1.0f,y+0.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z+1.0f);
	
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x+1.0f,y-1.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z+1.0f);
	
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z+0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z-1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x+1.0f,y-1.0f,z-1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y-1.0f,z+0.0f);
	
	//Bottom side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z-1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z-1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y-1.0f,z+1.0f);
	
	//Top side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+1.0f,z+1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y+1.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-0.0f,y+1.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-0.0f,y+1.0f,z+1.0f);
	
	//Midle side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z+0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z-1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y+0.0f,z-1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z+0.0f);
	
	//Top front side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,z+1.0f,z+0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z+0.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z+0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+1.0f,z+0.0f);
	
	//Bottom front side
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x+1.0f,y+0.0f,z-1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);glVertex3f(x+1.0f,y-1.0f,z-1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);glVertex3f(x-1.0f,y-1.0f,z-1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);glVertex3f(x-1.0f,y+0.0f,z-1.0f);
	
	glEnd();
	
	return 0;
}

/********************
* Bmp Texture Load
*
********************/

GLuint loadBMP_custom(const char * imagepath)
{

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize; 
	unsigned char * data;
	
//	imagepath = GetCurrentDirectory + imagepath;
	
	FILE * file = fopen(imagepath,"rb"); 
	if (!file) 
	{ 
		MessageBox(NULL,"Can't open image!'.","Texture BMP Load failed!",MB_OK | MB_ICONINFORMATION); 
		return 0; 
	}
	if ( fread(header, 1, 54, file) != 54 ) 
	{  
		MessageBox(NULL,"Incorrect BMP file!.","Texture BMP Load failed!",MB_OK | MB_ICONINFORMATION);
		return 0; 
	}
	if ( header[0]!='B' || header[1]!='M' )
	{ 
		MessageBox(NULL,"Incorrect BMP file!.","Texture BMP Load failed!",MB_OK | MB_ICONINFORMATION);
		return 0; 
	}

	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);
	if (imageSize==0) imageSize=width*height*3;
	if (dataPos==0) dataPos=54;
	data = new unsigned char [imageSize];
	fread(data,1,imageSize,file);
	fclose(file);
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	return textureID;
}


/*******************
* Blocks
*
*******************/
int StandartBlock(float x, float y, float z, float baseU, float baseV)
{
	SimpleCube(x,y,z,baseU,baseV);
	return 0;
}

int Layer3Block(float x, float y, float z, float baseU1, float baseV1, 
				float baseU2, float baseV2, float baseU3, float baseV3)
{
	glColor3f(1.0f,1.0f,1.0f);
	baseU1 = baseU1 / 64.0f;
	baseV1 = (64.0f - baseV1 - 1.0f) / 64.0f;
	
	baseU2 = baseU2 / 64.0f;
	baseV2 = (64.0f - baseV2 - 1.0f) / 64.0f;
	
	baseU3 = baseU3 / 64.0f;
	baseV3 = (64.0f - baseV3 - 1.0f) / 64.0f;
	glBegin (GL_QUADS);
	
	
	//Front side
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f);  	   glVertex3f (x-1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU2 + 0.0f,baseV2);              	   glVertex3f (x-1.0f, y-1.0f, z+1.0f);	
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 		       glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Top side
	glTexCoord2f(baseU1 + 1.0f/64.0f,baseV1); 			 glVertex3f (x+1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU1 + 0.0f,baseV1); 	 				 glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU1 + 0.0f,baseV1 + 1.0f/64.0f); 		 glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU1 + 1.0f/64.0f,baseV1 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	
	//Bottom side
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3 + 1.0f/64.0f); glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU3 + 0.0f,baseV3 + 1.0f/64.0f);		 glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU3 + 0.0f,baseV3);			  		 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3); 			 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	
	//Left side
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 	     	 glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 					 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 		 glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	//Right side
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 		 glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 		 	 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 					 glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Back side
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 		 glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 		   	 		 glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 			 glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	glEnd ();
	return 0;
}

int Layer3FrontBlock(float x, float y, float z, float baseU1, float baseV1, 
float baseU2, float baseV2, float baseU3, float baseV3)
{
		glColor3f(1.0f,1.0f,1.0f);
	baseU1 = baseU1 / 64.0f;
	baseV1 = (64.0f - baseV1 - 1.0f) / 64.0f;
	
	baseU2 = baseU2 / 64.0f;
	baseV2 = (64.0f - baseV2 - 1.0f) / 64.0f;
	
	baseU3 = baseU3 / 64.0f;
	baseV3 = (64.0f - baseV3 - 1.0f) / 64.0f;
	glBegin (GL_QUADS);
	
	
	//Front side
	glTexCoord2f(baseU1 + 1.0f/64.0f,baseV1 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU1 + 0.0f,baseV1 + 1.0f/64.0f);  	   glVertex3f (x-1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU1 + 0.0f,baseV1);              	   glVertex3f (x-1.0f, y-1.0f, z+1.0f);	
	glTexCoord2f(baseU1 + 1.0f/64.0f,baseV1); 		       glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Top side
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3); 			   glVertex3f (x+1.0f, y+1.0f, z+1.0f);	
	glTexCoord2f(baseU3 + 0.0f,baseV3); 	 			   glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU3 + 0.0f,baseV3 + 1.0f/64.0f); 	   glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	
	//Bottom side
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3 + 1.0f/64.0f); glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU3 + 0.0f,baseV3 + 1.0f/64.0f);	   glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU3 + 0.0f,baseV3);			  		   glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU3 + 1.0f/64.0f,baseV3); 			   glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	
	//Left side
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 	     	   glVertex3f (x-1.0f, y-1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 				   glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 	   glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	//Right side
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 	   glVertex3f (x+1.0f, y+1.0f, z+1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 		 	   glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 				   glVertex3f (x+1.0f, y-1.0f, z+1.0f);
	
	//Back side
	glTexCoord2f(baseU2 + 0.0f,baseV2 + 1.0f/64.0f); 	   glVertex3f (x+1.0f, y+1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 0.0f,baseV2); 		   	  	   glVertex3f (x+1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2); 			   glVertex3f (x-1.0f, y-1.0f, z-1.0f);
	glTexCoord2f(baseU2 + 1.0f/64.0f,baseV2 + 1.0f/64.0f); glVertex3f (x-1.0f, y+1.0f, z-1.0f);
	
	glEnd ();
	return 0;
}


int ChunkChange(int x, int y)
{
	if (x != 0 || y !=0)
	{
		while (x > 0)
		{
			for (int i=0; i<=2; i++)
			{
				data[w[i*3]].active = -1;
				SaveChunk(i*3);
			}
			for (int i=0; i<=2; i++)
			 for (int j=0; j<=1; j++)
			 {
			 	w[i*3+j] = w[i*3+(j+1)];
			 }
			for (int i=0; i<=2; i++)
			{
				w[i*3+2] = LoadChunk(targetx+2,targety+i-1);
			}  
			for (int i=0; i<=2; i++)
			{
				int r1,r2,r3,r4;
				GetNearestChunks(i*3+2, r1, r2, r3, r4, 3);
				OptimizeChunk(data[w[i*3+2]], data[w[r4]], data[w[r3]], data[w[r2]], data[w[r1]]);
			}
			x--;
		}
		while (x < 0)
		{
			for (int i=0; i<=2; i++)
			{
				data[w[i*3+2]].active = -1;
				SaveChunk(i*3+2);
			}
			for (int i=0; i<=2; i++)
			 for (int j=2; j>=1; j--)
			 {
			 	w[i*3+j] = w[i*3+(j-1)];
			 }
			for (int i=0; i<=2; i++)
			{
				w[i*3] = LoadChunk(targetx-2,targety+i-1);
			}  
			for (int i=0; i<=2; i++)
			{
				int r1,r2,r3,r4;
				GetNearestChunks(i*3, r1, r2, r3, r4, 3);
				OptimizeChunk(data[w[i*3]], data[w[r4]], data[w[r3]], data[w[r2]], data[w[r1]]);
			}
			x++;			
		}
		while (y>0)
		{
			for (int i=0; i<=2; i++)
			{
				data[w[i]].active = -1;
				SaveChunk(i);
			}
			for (int i=0; i<=2; i++)
			 for (int j=0; j<=1; j++)
			 {
			 	w[i+j*3] = w[i+(j+1)*3];
			 }
			for (int i=0; i<=2; i++)
			{
				w[i+6] = LoadChunk(targetx+i-1,targety+2);
			}  
			for (int i=0; i<=2; i++)
			{
				int r1,r2,r3,r4;
				GetNearestChunks(i+6, r1, r2, r3, r4, 3);
				OptimizeChunk(data[w[i+6]], data[w[r4]], data[w[r3]], data[w[r2]], data[w[r1]]);
			}
			y--;
		}
		while (y<0)
		{
			for (int i=0; i<=2; i++)
			{
				data[w[i+6]].active = -1;
				SaveChunk(i+6);
			}
			for (int i=0; i<=2; i++)
			 for (int j=1; j>=0; j--)
			 {
			 	w[i+(j+1)*3] = w[i+j*3] ;
			 }
			for (int i=0; i<=2; i++)
			{
				w[i] = LoadChunk(targetx+i-1,targety-2);
			}
			for (int i=0; i<=2; i++)
			{
				int r1,r2,r3,r4;
				GetNearestChunks(i, r1, r2, r3, r4, 3);
				OptimizeChunk(data[w[i]], data[w[r4]], data[w[r3]], data[w[r2]], data[w[r1]]);
			}  
			y++;
		}
	}
}

int GetSelectColorObject ( vector3ub &pixel )
{
	return pixel[0]*16+pixel[1]*64+pixel[2]*16;
}


void ProcessColorBuffer ( int x, int y )
{
  vector3ub pixel;
  glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		



  glDisable(GL_TEXTURE_2D);
  int count = 0;
  vector3ub color = {0};
	int px = data[w[4]].x * 32;
	int py = data[w[4]].y * 32;	

	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			for (int k = 0; k < 64; k++)
			{
				if (data[w[4]].status[i][j][k] != NoDraw)
				{
					switch (data[w[4]].map[i][j][k])
					{
					case 0: 	// Air
						break;  
					default: 	// Solid
					    SimpleCubePick(2*i+px,2*k,2*j+py, i/256.0f,k/256.0f,j/256.0f);
					}
				}
			}	

  glReadPixels ( x, y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel );
  CurrentObject = GetSelectColorObject (pixel);
  Select[0] = pixel[0]; //X
  Select[1] = pixel[1]; //Y
  Select[2] = pixel[2]; //Z
  glEnable(GL_TEXTURE_2D);
}





void DrawInterface(GLuint texture, int sizeX, int sizeY)
{
	glBindTexture(GL_TEXTURE_2D,texture); // биндим нужную нам текстуру
	for (int i = 0; i < 8; i++)
	{
		DrawInterfaceCell(i,0,0);
		if (Inventory[i] != NULL)
		switch (Inventory[i]->Id)
		{
			case 0:	DrawInterfaceCell(i,0,0);
				break;
			case 1: DrawInterfaceCell(i,1,0);
				break;
			case 2: DrawInterfaceCell(i,2,0);
				break;
			case 3: DrawInterfaceCell(i,3,0);
				break;	
			case 4: DrawInterfaceCell(i,4,0);	
				break;	
			case 5: DrawInterfaceCell(i,5,0);	
				break;	
		}
	}
}

void StartInterfaceDraw(int sizeX, int sizeY)
{
		glPushMatrix(); // сохран€ем GL_MODELVIEW
	glLoadIdentity(); 
	glMatrixMode( GL_PROJECTION ); // текуща€ - проекци€
	glPushMatrix(); // сохран€ем
	glLoadIdentity();
	// включаем 2D режим
	gluOrtho2D( 0, sizeX, sizeY, 0 );
	glMatrixMode( GL_MODELVIEW );
}

void EndInterfaceDraw()
{
			glMatrixMode( GL_PROJECTION );
			glPopMatrix(); // возвращаем GL_PROJECTION 
			glMatrixMode( GL_MODELVIEW );
			glPopMatrix(); // возвращаем GL_MODELVIEW
}



void DrawInterfaceCell(int i, float baseU, float baseV)
{
	baseU = baseU / 64.0f;
	baseV = (64.0f - baseV - 1.0f) / 64.0f;
	
	glBegin( GL_QUADS );
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex2f((i*64) + 224+34.0f, 676);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex2f((i*64) + 224 + 64+34.0f, 676);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex2f((i*64) + 224 + 64+34.0f, 740);
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex2f((i*64) + 224+34.0f, 740);
	glEnd();
}

void DrawHealthBar(GLuint texture,int Health, int sizeX, int sizeY)
{
	glBindTexture(GL_TEXTURE_2D,texture); // биндим нужную нам текстуру
	
	glBegin(GL_QUADS);
	glTexCoord2f(0,1); glVertex2f(258.0f, 645.0f);
	glTexCoord2f(1,1); glVertex2f((770.0f - 258.0f) * (Health/100.0f) + 258.0f, 645.0f);
	glTexCoord2f(1,0); glVertex2f((770.0f - 258.0f)* (Health/100.0f) + 258.0f, 660.0f);
	glTexCoord2f(0,0); glVertex2f(258.0f, 660.0f);
	glEnd();
}

void DrawInventoryBox(GLuint texture, int sizeX, int sizeY)
{
	glBindTexture(GL_TEXTURE_2D,texture);
	
	glBegin( GL_QUADS );
	glTexCoord2f(0,1); glVertex2f(244, 134);
	glTexCoord2f(1,1); glVertex2f(780, 134);
	glTexCoord2f(1,0); glVertex2f(780, 634);
	glTexCoord2f(0,0); glVertex2f(244, 634);
	glEnd();
}

void DrawInventory(GLuint texture, int sizeX, int sizeY)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	
	for (int i = 0; i < 32; i++)
	{
	    DrawInventoryCell(i%8,4-(i/8),0,0);
	    if (Inventory[i] != NULL)
		switch(Inventory[i]->Id)
		{
			case 0:	DrawInventoryCell(i%8,4-(i/8),0,0);
				break;
			case 1: DrawInventoryCell(i%8,4-(i/8),1,0);
				break;
			case 2: DrawInventoryCell(i%8,4-(i/8),2,0);
				break;
			case 3: DrawInventoryCell(i%8,4-(i/8),3,0);
				break;	
			case 4: DrawInventoryCell(i%8,4-(i/8),4,0);	
				break;
			case 5: DrawInventoryCell(i%8,4-(i/8),5,0);	
				break;				
				 
		}
	}	
}

void DrawItemOnCursor(GLuint texture, int sizeX, int sizeY, int x, int y, Item* TakenItem)
{
	glBindTexture(GL_TEXTURE_2D, texture);
    if (TakenItem != NULL)
	switch(TakenItem->Id)
	{
		case 0:	DrawItem(x,y,0,0);
			break;
		case 1: DrawItem(x,y,1,0);
			break;
		case 2: DrawItem(x,y,2,0);
			break;
		case 3: DrawItem(x,y,3,0);
			break;	
		case 4: DrawItem(x,y,4,0);	
			break;
		case 5: DrawItem(x,y,5,0);	
			break;			 
	}
}

void DrawInventoryCell(int i, int j, float baseU, float baseV)
{
	baseU = baseU / 64.0f;
	baseV = (64.0f - baseV - 1.0f) / 64.0f;
	
	glBegin(GL_QUADS);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex2f((i*64) + 260,(j*64) + 310);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex2f((i*64) + 260 + 64,(j*64) + 310);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex2f((i*64) + 260 + 64, (j*64) + 374);
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex2f((i*64) + 260, (j*64) + 374);
	glEnd();
}

void DrawItem(int x, int y, float baseU, float baseV)
{
	baseU = baseU / 64.0f;
	baseV = (64.0f - baseV - 1.0f) / 64.0f;
	
	glBegin(GL_QUADS);
	glTexCoord2f(baseU + 0.0f,baseV + 1.0f/64.0f); 		 glVertex2f(x,y);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV + 1.0f/64.0f); glVertex2f(x + 64,y);
	glTexCoord2f(baseU + 1.0f/64.0f,baseV); 			 glVertex2f(x + 64, y+64);
	glTexCoord2f(baseU + 0.0f,baseV); 					 glVertex2f(x, y+64);
	glEnd();
}

void InitInventoryTest() // Load test items
{
	for (int i=0; i<32; i++)
		{
			Inventory[i] = new ItemResource(0+(rand()%6),i+1);
		}
}

int SaveChunk(int target)
{
	char str[255] = "map/m(";
 	char str1[32];
 	sprintf(str1, "%d", data[w[target]].x);
 	strcat(str,str1);
 	strcat(str, ",");
    sprintf(str1, "%d", data[w[target]].y);
    strcat(str,str1);
    strcat(str,").map");
    
	FILE *fp;
	if((fp=fopen(str, "wb"))==NULL) 
	{
	    printf("Ќе удаетс€ открыть файл.\n");
	    exit(1);
    }
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			for (int k = 0; k < 64; k++)
				fwrite(&data[w[target]].map[i][j][k], sizeof(int), 1, fp);				    
	fclose(fp);
	return 0;
}
