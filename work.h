#include <math.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <fstream>

using namespace std;

class tChunk
{
	public:
		int active;
		int x,y;
		int map[16][16][64];
		int status[16][16][64];
};

tChunk world;
tChunk data[maxChunk];

const int maxChunk = 20;

int w[9];
int ChunkTop = -1;
int targetx, targety = {0};

/* BIT MASK */
const int NoDraw = 2;

/*************************
 * Spherical Coordinate System
 *
 ************************/
 
void CartToSphere(float x, float z, float y, float &r, float &t, float &p)
{
	r = sqrt(x*x+y*y+z*z);
	t = atan(sqrt(x*x+y*y)/z);
	p = atan(y/x);
}

void SphereToCart(const float r, const float t, const float p, float &x, float &y, float &z)
{
	x = r*sin (t)*cos (p);
	z = r*sin (t)*sin (p);
	y = r*cos (t);
}

const double EPS = 1E-9;
 
struct pt 
{
	double x, y;
 
	bool operator< (const pt & p) const 
	{
		return x < p.x-EPS || abs(x-p.x) < EPS && y < p.y - EPS;
	}
};
 
struct line 
{
	double a, b, c;
 
	line() {}
	line (pt p, pt q) 
	{
		a = p.y - q.y;
		b = q.x - p.x;
		c = - a * p.x - b * p.y;
		norm();
	}
 
	void norm() 
	{
		double z = sqrt (a*a + b*b);
		if (abs(z) > EPS)
			a /= z,  b /= z,  c /= z;
	}
 
	double dist (pt p) const 
	{
		return a * p.x + b * p.y + c;
	}
};
 
#define det(a,b,c,d)  (a*d-b*c)
 
inline bool betw (double l, double r, double x) 
{
	return min(l,r) <= x + EPS && x <= max(l,r) + EPS;
}
 
inline bool intersect_1d (double a, double b, double c, double d) 
{
	if (a > b)  swap (a, b);
	if (c > d)  swap (c, d);
	return max (a, c) <= min (b, d) + EPS;
}
 
bool intersect (pt a, pt b, pt c, pt d, pt & left, pt & right) 
{
	if (! intersect_1d (a.x, b.x, c.x, d.x) || ! intersect_1d (a.y, b.y, c.y, d.y))
		return false;
	line m (a, b);
	line n (c, d);
	double zn = det (m.a, m.b, n.a, n.b);
	if (abs (zn) < EPS) 
	{
		if (abs (m.dist (c)) > EPS || abs (n.dist (a)) > EPS)
			return false;
		if (b < a)  swap (a, b);
		if (d < c)  swap (c, d);
		left = max (a, c);
		right = min (b, d);
		return true;
	}
	else 
	{
		left.x = right.x = - det (m.c, m.b, n.c, n.b) / zn;
		left.y = right.y = - det (m.a, m.c, n.a, n.c) / zn;
		return betw (a.x, b.x, left.x)
			&& betw (a.y, b.y, left.y)
			&& betw (c.x, d.x, left.x)
			&& betw (c.y, d.y, left.y);
	}
}
 


void CollisionCheck(float oldx, float oldy, float oldz, float &newx, float &newy, float &newz) /* Calcs Collision with Blocks */
{
	// Preparing block location`
	float 	fx = ((newx-world.x+1)/2);
	float	fy = ((newy-1)/2);
	float	fz = ((newz-world.y+1)/2);
	int x = trunc(fx);
	int y = trunc(fy);
	int z = trunc(fz);
	// Locating point
	// Firstly we should find intersection point
	if (world.map[x][z][y] != 0 || world.map[x][z][y+1] != 0)
		{
			pt a = {oldx, oldz};                                 
			pt b = {newx, newz};								
			pt c = {(2*x)-1.0f, (2*z)-1.0f};							
			pt d = {(2*x)+1.0f, (2*z)-1.0f};								
			pt resLeft;
			pt resRight;
			bool RecFound = false;

			if (intersect(a,b,c,d,resLeft,resRight))				// If point located we should destroy one 
			{														// of coordinate components, to cancel movement
				// P1 - P2 Intersection
				newz = resLeft.y - 0.0001f; // Destroy Z component
				RecFound = true;
			}
			c.x = (2*x)+1.0f; c.y = (2*z)-1.0f;
			d.x = (2*x)+1.0f; d.y = (2*z)+1.0f;
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P2 - P3 Intersection
				newx = resLeft.x + 0.0001f;	// Destroy X component
				RecFound = true;
			}
			c.x = 2*x+1; c.y = 2*z+1;
			d.x = 2*x-1; d.y = 2*z+1;
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P3 - P4 Intersection
				newz = resLeft.y + 0.0001f;	// Destroy Z component
				RecFound = true;
			} 
			c.x = 2*x-1; c.y = 2*z+1;
			d.x = 2*x-1; d.y = 2*z-1;
			
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P4 - P1 Intersection
				newx = resLeft.x - 0.0001f;	// Destroy X component
				RecFound = true;
			}
			if (RecFound)
			{
				CollisionCheck( oldx, oldy, oldz, newx, newy, newz);
			}
		}
}




void CollisionCheckMulti(float oldx, float oldy, float oldz, float &newx, float &newy, float &newz) /* Calcs Collision with Blocks */
{
	// Preparing block location`
	float 	fx = ((newx+1)/2);
	float	fy = ((newy-1)/2);
	float	fz = ((newz+1)/2);
	int x = trunc(fx) - data[w[4]].x*16;
	int y = trunc(fy);
	int z = trunc(fz) - data[w[4]].y*16;
	if (fx < 0)
		x--;
	if (fz < 0)
		z--;
	// Locating point
	// Firstly we should find intersection point
	if ((x > 15 && z > 15)||(x > 15 && z < 0)||(x < 0 && z > 15)||(x < 0 && z < 0))
		return;
if (
	(data[w[4]].map[x][z][y] != 0 || data[w[4]].map[x][z][y+1] != 0) ||                       
 	( (x > 15 && (data[w[5]].map[0][z][y] != 0 || data[w[5]].map[0][z][y+1] != 0)) ||  /* Left side [5 chunk] */
	  (x < 0 && (data[w[3]].map[15][z][y] != 0 || data[w[3]].map[15][z][y+1] != 0)) || /* Right side [3 chunk] */
	  (z > 15 && (data[w[7]].map[x][0][y] != 0 || data[w[7]].map[x][0][y+1] != 0)) ||  /* Top side [7 chunk]*/
	  (z < 0 && (data[w[1]].map[x][15][y] != 0 || data[w[1]].map[x][15][y+1] != 0))     /* Bottom side [1 chunk]*/
	)
   )
		{	
			x = x + data[w[4]].x*16;
			z = z + data[w[4]].y*16;
			pt a = {oldx, oldz};                                 
			pt b = {newx, newz};								
			pt c = {(2*x)-1.0f, (2*z)-1.0f};							
			pt d = {(2*x)+1.0f, (2*z)-1.0f};								
			pt resLeft;
			pt resRight;
			bool RecFound = false;

			if (intersect(a,b,c,d,resLeft,resRight))				// If point located we should destroy one 
			{														// of coordinate components, to cancel movement
				// P1 - P2 Intersection
				newz = resLeft.y - 0.0001f; // Destroy Z component
				RecFound = true;
			}
			c.x = (2*x)+1.0f; c.y = (2*z)-1.0f;
			d.x = (2*x)+1.0f; d.y = (2*z)+1.0f;
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P2 - P3 Intersection
				newx = resLeft.x + 0.0001f;	// Destroy X component
				RecFound = true;
			}
			c.x = 2*x+1; c.y = 2*z+1;
			d.x = 2*x-1; d.y = 2*z+1;
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P3 - P4 Intersection
				newz = resLeft.y + 0.0001f;	// Destroy Z component
				RecFound = true;
			} 
			c.x = 2*x-1; c.y = 2*z+1;
			d.x = 2*x-1; d.y = 2*z-1;
			
			if (intersect(a,b,c,d,resLeft,resRight))
			{
				// P4 - P1 Intersection
				newx = resLeft.x - 0.0001f;	// Destroy X component
				RecFound = true;
			}
			if (RecFound)
			{
				CollisionCheckMulti( oldx, oldy, oldz, newx, newy, newz);
			}
		}
}

void CollisionCheckDiagonal(float oldx, float oldy, float oldz, float &newx, float &newy, float &newz)
{
	float	edge = 0.99999f;
	float 	fx = ((newx+1)/2); /* NEW */
	float	fy = ((newy-1)/2);
	float	fz = ((newz+1)/2);
	int x = trunc(fx) - data[w[4]].x*16;;
	int y = trunc(fy);
	int z = trunc(fz) - data[w[4]].y*16;
	if (fx < 0)
		x--;
	if (fz < 0)
		z--;
	float 	ofx = ((oldx+1)/2); /* OLD */
	float	ofy = ((oldy-1)/2);
	float	ofz = ((oldz+1)/2);
	int ox = trunc(ofx) - data[w[4]].x*16;
	int oy = trunc(ofy);
	int oz = trunc(ofz) - data[w[4]].y*16;
	if (ofx < 0)
		ox--;
	if (ofz < 0)
		oz--;	
	if (x > 15 || x < 0 || z > 15 || z < 0)	
	{
		if ((x > 15 && z > 15) && 
		((data[w[8]].map[0][0][y] != 0) || ((data[w[7]].map[15][0][y] != 0) && (data[w[5]].map[0][15][y] != 0)))
		)
			{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
			}
		if ((x < 0 && z > 15) && 
		((data[w[6]].map[15][0][y] != 0) || ((data[w[7]].map[0][0][y] != 0) && (data[w[3]].map[15][15][y] != 0)))
		)
			{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
			}
		if ((x < 0 && z < 0) && 
		((data[w[0]].map[15][15][y] != 0) || ((data[w[1]].map[0][15][y] != 0) && (data[w[3]].map[15][0][y] != 0)))
		)
			{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
			}			
		if ((x > 15 && z < 0) && 
		((data[w[2]].map[0][15][y] != 0) || ((data[w[1]].map[15][15][y] != 0) && (data[w[5]].map[0][0][y] != 0)))
		)
			{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
			}	
		if (x>15 && 
		((oz+1 == z && ((data[w[5]].map[0][z][y] != 0) || ((data[w[5]].map[0][z-1][y] != 0)&&(data[w[4]].map[15][z][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
		}
		if(x>15 && 
		((oz-1 == z && ((data[w[5]].map[0][z][y] != 0) || ((data[w[5]].map[0][z+1][y] != 0)&&(data[w[4]].map[15][z][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
		}
		if (x<0 && 
		((oz+1 == z && ((data[w[3]].map[15][z][y] != 0) || ((data[w[3]].map[15][z-1][y] != 0)&&(data[w[4]].map[0][z][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
		}
		if(x<0 && 
		((oz-1 == z && ((data[w[3]].map[15][z][y] != 0) || ((data[w[3]].map[15][z+1][y] != 0)&&(data[w[4]].map[0][z][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
		}
		if (z<0 && 
		((ox+1 == x && ((data[w[1]].map[x][15][y] != 0) || ((data[w[1]].map[x-1][15][y] != 0)&&(data[w[4]].map[x][0][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
		}
		if(z<0 && 
		((ox-1 == x && ((data[w[1]].map[x][15][y] != 0) || ((data[w[1]].map[x+1][15][y] != 0)&&(data[w[4]].map[x][0][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;	
			return;
		}
		if (z>15 && 
		((ox+1 == x && ((data[w[7]].map[x][15][y] != 0) || ((data[w[7]].map[x-1][0][y] != 0)&&(data[w[4]].map[x][15][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
		}
		if(z>15 && 
		((ox-1 == x && ((data[w[1]].map[x][15][y] != 0) || ((data[w[1]].map[x+1][0][y] != 0)&&(data[w[4]].map[x][15][y] != 0))))))
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;	
			return;
		}		
	}
	else
	if (x != ox && z != oz)
	{
		if (ox+1 == x && oz+1 == z && data[w[4]].map[ox+1][oz][oy] != 0 && data[w[4]].map[ox][oz+1][oy] != 0) 
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;
			return;
		}
		if (ox-1 == x && oz+1 == z && data[w[4]].map[ox-1][oz][oy] != 0 && data[w[4]].map[ox][oz+1][oy] != 0) 
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 + edge;
			return;
		}
		if (ox-1 == x && oz-1 == z && data[w[4]].map[ox-1][oz][oy] != 0 && data[w[4]].map[ox][oz-1][oy] != 0) 
		{
			newx = (ox+data[w[4]].x*16)*2 - edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;
			return;
		}
		if (ox+1 == x && oz-1 == z && data[w[4]].map[ox+1][oz][oy] != 0 && data[w[4]].map[ox][oz-1][oy] != 0) 
		{
			newx = (ox+data[w[4]].x*16)*2 + edge;
			newz = (oz+data[w[4]].y*16)*2 - edge;
			return;
		}
	}
}

void OptimizeChunk(tChunk &target, tChunk t1, tChunk t2, tChunk t3, tChunk t4)
{
	/* t1 -1 0; t2 0 -1; t3 +1 0; t4 0 +1 */
	/*       ****t4****			z
	 *       *        *			*
	 *       t3  Trg  t1        *      
	 *       *        *	   x*****
	 *       ****t2****                       */
	for (int i = 1; i < 15; i++)
		for (int j = 1; j < 15; j++)
			for (int k = 1; k < 63; k++)
			{
				bool NDraw = true;
				for (int i2 = i-1; i2 <= i+1; i2+=2)
					if (target.map[i2][j][k] == 0)
						NDraw = false;
				for (int j2 = j-1; j2 <= j+1; j2+=2)
					if (target.map[i][j2][k] == 0)
						NDraw = false;
				for (int k2 = k-1; k2 <= k+1; k2+=2)
					if (target.map[i][j][k2] == 0)
						NDraw = false;
				if (NDraw)
					target.status[i][j][k] = NoDraw;		
			}
	
	if (t1.status[0][0][0] != -1)
	{
		for (int k = 1; k < 63; k++)
			for (int j = 1; j < 15; j++)
			{
				if (!(t1.map[15][j][k] == 0 || target.map[1][j][k] == 0 || target.map[0][j-1][k] == 0  || target.map[0][j+1][k] == 0 || target.map[0][j][k-1] == 0 || target.map[0][j][k+1] == 0))
					target.status[0][j][k] = NoDraw;
			}
	}
	if (t2.status[0][0][0] != -1)
	{
		for (int k = 1; k < 63; k++)
			for (int j = 1; j < 15; j++)
			{
				if (!(t2.map[j][15][k] == 0 || target.map[j][1][k] == 0 || target.map[j-1][0][k] == 0  || target.map[j+1][0][k] == 0 || target.map[j][0][k-1] == 0 || target.map[j][0][k+1] == 0))
					target.status[j][0][k] = NoDraw;
			}	
	}
	if (t3.status[0][0][0] != -1)
	{
		for (int k = 1; k < 63; k++)
			for (int j = 1; j < 15; j++)
			{
				if (!(t3.map[0][j][k] == 0 || target.map[14][j][k] == 0 || target.map[15][j-1][k] == 0  || target.map[15][j+1][k] == 0 || target.map[15][j][k-1] == 0 || target.map[15][j][k+1] == 0))
					target.status[15][j][k] = NoDraw;
			}
	}
	if (t4.status[0][0][0] != -1)
	{
		for (int k = 1; k < 63; k++)
			for (int j = 1; j < 15; j++)
			{
				if (!(t4.map[j][0][k] == 0 || target.map[j][14][k] == 0 || target.map[j-1][15][k] == 0  || target.map[j+1][15][k] == 0 || target.map[j][15][k-1] == 0 || target.map[j][15][k+1] == 0))
					target.status[j][15][k] = NoDraw;
			}
	}
	
	
	if (t1.status[0][0][0] == -1 && t2.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
			if (!(target.map[15][15][i-1] == 0 || target.map[15][15][i+1] == 0 || target.map[14][15][i] == 0 || target.map[15][14][i] == 0 || t3.map[0][15][i] == 0 || t4.map[15][0][i] == 0))
				target.status[15][15][i] = NoDraw;
	}
	if (t2.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
		{
			if (!(target.map[0][15][i-1] == 0 || target.map[0][15][i+1] == 0 || target.map[1][15][i] == 0 || target.map[0][14][i] == 0 || t1.map[15][15][i] == 0 || t4.map[0][0][i] == 0))
				target.status[0][15][i] = NoDraw;
			if (!(target.map[15][15][i-1] == 0 || target.map[15][15][i+1] == 0 || target.map[14][15][i] == 0 || target.map[15][14][i] == 0 || t3.map[0][15][i] == 0 || t4.map[15][0][i] == 0))
				target.status[15][15][i] = NoDraw;
		}
	}
	if (t2.status[0][0][0] == -1 && t3.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
			if (!(target.map[0][15][i-1] == 0 || target.map[0][15][i+1] == 0 || target.map[1][15][i] == 0 || target.map[0][14][i] == 0 || t1.map[15][15][i] == 0 || t4.map[0][0][i] == 0))
				target.status[0][15][i] = NoDraw;
	}
	if (t3.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
		{
			if (!(target.map[0][0][i-1] == 0 || target.map[0][0][i+1] == 0 || target.map[1][0][i] == 0 || target.map[0][1][i] == 0 || t1.map[15][0][i] == 0 || t2.map[0][15][i] == 0))
				target.status[0][0][i] = NoDraw;
			if (!(target.map[0][15][i-1] == 0 || target.map[0][15][i+1] == 0 || target.map[1][15][i] == 0 || target.map[0][14][i] == 0 || t1.map[15][15][i] == 0 || t4.map[0][0][i] == 0))
				target.status[0][15][i] = NoDraw;
		}
	}
	if (t3.status[0][0][0] == -1 && t4.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
			if (!(target.map[0][0][i-1] == 0 || target.map[0][0][i+1] == 0 || target.map[1][0][i] == 0 || target.map[0][1][i] == 0 || t1.map[15][0][i] == 0 || t2.map[0][15][i] == 0))
				target.status[0][0][i] = NoDraw;
	}
	if (t4.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
		{
			if (!(target.map[15][0][i-1] == 0 || target.map[15][0][i+1] == 0 || target.map[14][0][i] == 0 || target.map[15][1][i] == 0 || t2.map[15][15][i] == 0 || t3.map[0][0][i] == 0))
				target.status[15][0][i] = NoDraw;
			if (!(target.map[0][0][i-1] == 0 || target.map[0][0][i+1] == 0 || target.map[1][0][i] == 0 || target.map[0][1][i] == 0 || t1.map[15][0][i] == 0 || t2.map[0][15][i] == 0))
				target.status[0][0][i] = NoDraw;
		}
	}
	if (t1.status[0][0][0] == -1 && t4.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
			if (!(target.map[15][0][i-1] == 0 || target.map[15][0][i+1] == 0 || target.map[14][0][i] == 0 || target.map[15][1][i] == 0 || t2.map[0][15][i] == 0 || t3.map[0][0][i] == 0))
				target.status[15][0][i] = NoDraw;
	}
	if (t1.status[0][0][0] == -1)
	{
		for (int i = 1; i < 63; i++)
		{
			if (!(target.map[15][15][i-1] == 0 || target.map[15][15][i+1] == 0 || target.map[14][15][i] == 0 || target.map[15][14][i] == 0 || t3.map[0][15][i] == 0 || t4.map[15][0][i] == 0))
				target.status[15][15][i] = NoDraw;
			if (!(target.map[15][0][i-1] == 0 || target.map[15][0][i+1] == 0 || target.map[14][0][i] == 0 || target.map[15][1][i] == 0 || t3.map[0][0][i] == 0 || t2.map[15][15][i] == 0))
				target.status[15][0][i] = NoDraw;
		}
	}
	if (t1.status[0][0][0] != -1 && t2.status[0][0][0] != -1 && t3.status[0][0][0] != -1 && t4.status[0][0][0] != -1)
	{
		for (int i = 1; i < 63; i++)
		{
			if (!(target.map[0][0][i-1] == 0 || target.map[0][0][i+1] == 0 || target.map[1][0][i] == 0 || target.map[0][1][i] == 0 || t1.map[15][0][i] == 0 || t2.map[0][15][i] == 0))
				target.status[0][0][i] = NoDraw;
			if (!(target.map[15][0][i-1] == 0 || target.map[15][0][i+1] == 0 || target.map[14][0][i] == 0 || target.map[15][1][i] == 0 || t3.map[0][0][i] == 0 || t2.map[15][15][i] == 0))
				target.status[15][0][i] = NoDraw;
			if (!(target.map[15][15][i-1] == 0 || target.map[15][15][i+1] == 0 || target.map[14][15][i] == 0 || target.map[15][14][i] == 0 || t3.map[0][15][i] == 0 || t4.map[15][0][i] == 0))
				target.status[15][15][i] = NoDraw;
			if (!(target.map[0][15][i-1] == 0 || target.map[0][15][i+1] == 0 || target.map[1][15][i] == 0 || target.map[0][14][i] == 0 || t4.map[0][0][i] == 0 || t1.map[15][15][i] == 0))
				target.status[0][15][i] = NoDraw;
		}
	}
	//target.status[x][z][y] || NoDraw;
}

void GetNearestChunks(int target, int &Top, int &Left, int &Bottom, int &Right, int StrokeLong)
{
	//Left
	if ((target + 1) % StrokeLong != 0)
		Left = target + 1;
	else Left = -1;
	//Right
	if (target % StrokeLong != 0)
		Right = target - 1;
	else Right = -1;
	//Top
	if (target + StrokeLong < (StrokeLong*StrokeLong))
		Top = target + StrokeLong;
	else Top = -1;
	//Bottom
	if (target - StrokeLong >= 0)
		Bottom = target - StrokeLong;
	else Bottom = -1;
		
}

int GetCentralChunk(int StrokeLong)
{
	return ((StrokeLong*StrokeLong)-1)/2;
}

void CheckBorder(float borderx, float borderz, float &positionx, float &positionz) /* Calcs Collision with Borders */
{
	if (positionx > borderx)
		positionx = borderx-0.001;
	if (positionz > borderz)
		positionz = borderz-0.001;	
	if (positionx < -1)
		positionx = -0.999;
	if (positionz < -1)
		positionz = -0.999;
}

void CheckChunk(float newx, float newy, float newz, int &dx, int &dy)
{
	float 	fx = ((newx+1)/2);
	float	fy = ((newy-1)/2);
	float	fz = ((newz+1)/2);
	int x = trunc(fx) - data[w[4]].x*16;
	int y = trunc(fy);
	int z = trunc(fz) - data[w[4]].y*16;
	if (fx < 0)
		x--;
	if (fz < 0)
		z--;
	if (x>15)
	 dx = 1;
	else dx = 0; 
	if (z>15)
	 dy = 1;
	else dy = 0; 
	if (x<0)
	 dx = -1;
	if (z<0)
	 dy = -1;
}



void CheckStartPosition(float &objy, float objx, float objz, tChunk target)
{
	float 	fx = ((objx+1)/2);
	float	fy = ((objy-1)/2);
	float	fz = ((objz+1)/2);
	int x = trunc(fx) - data[w[4]].x*16;
	int y = trunc(fy);
	int z = trunc(fz) - data[w[4]].y*16;
	if (fx < 0)
		x--;
	if (fz < 0)
		z--;
	
	while(data[w[4]].map[x][z][y] != 0)
	{
		objy += 2.0f;
		float fy = ((objy-2.75f)/2);
		y = trunc(fy);
	}
}

void BlockDeletedInsideChunk(tChunk &target,int x, int y, int z)
{
	target.status[x-1][z][y] = 0;
	target.status[x+1][z][y] = 0;
	target.status[x][z-1][y] = 0;
	target.status[x][z+1][y] = 0;
	target.status[x][z][y-1] = 0;
	target.status[x][z][y+1] = 0;
}

void BlockDeletedBorderChunk(tChunk &target, tChunk &neighboor , int x, int y, int z)
{
	if (x == 0)
	{
		neighboor.status[15][z][y] = 0;
		target.status[x+1][z][y] = 0;
		target.status[x][z-1][y] = 0;
		target.status[x][z+1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (x == 15)
	{
		neighboor.status[0][z][y] = 0;
		target.status[x-1][z][y] = 0;
		target.status[x][z-1][y] = 0;
		target.status[x][z+1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (z == 0)
	{
		neighboor.status[x][15][y] = 0;
		target.status[x-1][z][y] = 0;
		target.status[x+1][z][y] = 0;
		target.status[x][z+1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (z == 15)
	{
		neighboor.status[x][0][y] = 0;
		target.status[x-1][z][y] = 0;
		target.status[x+1][z][y] = 0;
		target.status[x][z-1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
}

void BlockDeletedAngleChunk(tChunk &target, tChunk &t1, tChunk &t2, int x, int y, int z)
{
	if (x == 0 && z == 0)
	{
		t1.status[15][0][y] = 0;
		t2.status[0][15][y] = 0;
		target.status[x+1][z][y] = 0;
		target.status[x][z+1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (x == 15 && z == 0)
	{
		t1.status[15][15][y] = 0;
		t2.status[0][0][y] = 0;
		target.status[x-1][z][y] = 0;
		target.status[x][z+1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (x == 0 && z == 15)
	{
		t1.status[0][0][y] = 0;
		t2.status[15][15][y] = 0;
		target.status[x+1][z][y] = 0;
		target.status[x][z-1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
	if (x == 15 && z == 15)
	{
		t1.status[0][15][y] = 0;
		t2.status[15][0][y] = 0;
		target.status[x-1][z][y] = 0;
		target.status[x][z-1][y] = 0;
		target.status[x][z][y-1] = 0;
		target.status[x][z][y+1] = 0;
	}
}

void BlockCreatedInsideChunk(tChunk &target, int x, int y, int z)
{
	if (target.map[x-2][z][y] != 0 && target.map[x-1][z-1][y] != 0 && target.map[x-1][z+1][y] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x-1][z][y+1] != 0)
		target.status[x-1][z][y] = NoDraw;
		
	if (target.map[x+2][z][y] != 0 && target.map[x+1][z-1][y] != 0 && target.map[x+1][z+1][y] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x+1][z][y+1] != 0)
		target.status[x+1][z][y] = NoDraw;
		
	if (target.map[x][z-2][y] != 0 && target.map[x-1][z-1][y] != 0 && target.map[x+1][z-1][y] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z-1][y+1] != 0)
		target.status[x][z-1][y] = NoDraw;
		
	if (target.map[x][z+2][y] != 0 && target.map[x-1][z+1][y] != 0 && target.map[x+1][z+1][y] != 0 && target.map[x][z+1][y-1] != 0 && target.map[x][z+1][y+1] != 0)
		target.status[x][z+1][y] = NoDraw;
		
	if (target.map[x][z][y-2] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z+1][y-1] != 0)
		target.status[x][z][y-1] = NoDraw;
		
	if (target.map[x][z][y+2] != 0 && target.map[x-1][z][y+1] != 0 && target.map[x+1][z][y+1] != 0 && target.map[x][z-1][y+1] != 0 && target.map[x][z+1][y+1] != 0)
		target.status[x][z][y+1] = NoDraw;
}

void BlockCreatedBorderChunk(tChunk &target, tChunk &neighboor, int x, int y, int z)
{
	if (x == 0)
	{
		if (neighboor.map[14][z][y] != 0 && neighboor.map[15][z-1][y] != 0 && neighboor.map[15][z+1][y] != 0 && neighboor.map[15][z][y-1] != 0 && neighboor.map[15][z][y+1] != 0)
			neighboor.status[15][z][y] = NoDraw;
		
		if (target.map[x+2][z][y] != 0 && target.map[x+1][z-1][y] != 0 && target.map[x+1][z+1][y] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x+1][z][y+1] != 0)
			target.status[x+1][z][y] = NoDraw;
		
		if (target.map[x][z-2][y] != 0 && neighboor.map[15][z-1][y] != 0 && target.map[x+1][z-1][y] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z-1][y+1] != 0)
			target.status[x][z-1][y] = NoDraw;
		
		if (target.map[x][z+2][y] != 0 && neighboor.map[15][z-1][y] != 0 && target.map[x+1][z+1][y] != 0 && target.map[x][z+1][y-1] != 0 && target.map[x][z+1][y+1] != 0)
			target.status[x][z+1][y] = NoDraw;
		
		if (target.map[x][z][y-2] != 0 && neighboor.map[15][z][y-1] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z+1][y-1] != 0)
			target.status[x][z][y-1] = NoDraw;
		
		if (target.map[x][z][y+2] != 0 && neighboor.map[15][z][y+1] != 0 && target.map[x+1][z][y+1] != 0 && target.map[x][z-1][y+1] != 0 && target.map[x][z+1][y+1] != 0)
			target.status[x][z][y+1] = NoDraw;
	}
	if (x == 15)
	{
		if (neighboor.map[1][z][y] != 0 && neighboor.map[0][z-1][y] != 0 && neighboor.map[0][z+1][y] != 0 && neighboor.map[0][z][y-1] && neighboor.map[0][z][y+1] != 0)
			neighboor.status[0][z][y] = NoDraw;
			
		if (target.map[x-2][z][y] != 0 && target.map[x-1][z-1][y] != 0 && target.map[x-1][z+1][y] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x-1][z][y+1] != 0)
			target.status[x-1][z][y] = NoDraw;
		
		if (target.map[x][z-2][y] != 0 && neighboor.map[0][z-1][y] != 0 && target.map[x-1][z-1][y] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z-1][y+1] != 0)
			target.status[x][z-1][y] = NoDraw;
		
		if (target.map[x][z+2][y] != 0 && neighboor.map[0][z+1][y] != 0 && target.map[x-1][z+1][y] != 0 && target.map[x][z+1][y-1] != 0 && target.map[x][z+1][y+1] != 0)
			target.status[x][z+1][y] = NoDraw;
		
		if (target.map[x][z][y-2] != 0 && neighboor.map[0][z][y-1] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x][z-1][y-1] != 0 && target.map[x][z+1][y-1] != 0)
			target.status[x][z][y-1] = NoDraw;
		
		if (target.map[x][z][y+2] != 0 && neighboor.map[0][z][y+1] != 0 && target.map[x-1][z][y+1] != 0 && target.map[x][z-1][y+1] != 0 && target.map[x][z+1][y+1] != 0)
			target.status[x][z][y+1] = NoDraw;
	}
	if (z == 0)
	{
		if (neighboor.map[x][14][y] != 0 && neighboor.map[x-1][15][y] != 0 && neighboor.map[x+1][15][y] != 0 && neighboor.map[x][15][y-1] != 0 && neighboor.map[x][15][y+1] != 0)
			neighboor.status[x][15][y] = NoDraw;
			
		if (target.map[x-1][z][y] != 0 && target.map[x+1][z][y] != 0 && target.map[x][z+2][y] != 0 && target.map[x][z][y-1] != 0 && target.map[x][z][y+1] != 0)
			target.status[x][z+1][y] = NoDraw;
			
		if (target.map[x+2][z][y] != 0 && neighboor.map[x+1][15][y] != 0 && target.map[x+1][z+1][y] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x+1][z][y+1] != 0)
			target.status[x+1][z][y] = NoDraw;
		
		if (target.map[x-2][z][y] != 0 && neighboor.map[x-1][15][y] != 0 && target.map[x-1][z+1][y] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x-1][z][y+1] != 0)
			target.status[x-1][z][y] = NoDraw;
		
		if (target.map[x][z][y+2] != 0 && neighboor.map[x][15][y+1] != 0 && target.map[x-1][z][y+1] != 0 && target.map[x+1][z][y+1] && target.map[x][z+1][y+1] != 0)
			target.status[x][z][y+1] = NoDraw;
			
		if (target.map[x][z][y-2] != 0 && neighboor.map[x][15][y-1] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x+1][z][y-1] && target.map[x][z+1][y-1] != 0)
			target.status[x][z][y-1] = NoDraw;		
	}
	if (z == 15)
	{
		if (neighboor.map[x][1][y] != 0 && neighboor.map[x-1][0][y] != 0 && neighboor.map[x+1][0][y] != 0 && neighboor.map[x][0][y-1] != 0 && neighboor.map[x][0][y+1] != 0)
			neighboor.status[x][15][y] = NoDraw;
			
		if (target.map[x-1][z][y] != 0 && target.map[x+1][z][y] != 0 && target.map[x][z-2][y] != 0 && target.map[x][z][y-1] != 0 && target.map[x][z][y+1] != 0)
			target.status[x][z-1][y] = NoDraw;
			
		if (target.map[x+2][z][y] != 0 && neighboor.map[x+1][0][y] != 0 && target.map[x+1][z-1][y] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x+1][z][y+1] != 0)
			target.status[x+1][z][y] = NoDraw;
		
		if (target.map[x-2][z][y] != 0 && neighboor.map[x-1][0][y] != 0 && target.map[x-1][z-1][y] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x-1][z][y+1] != 0)
			target.status[x-1][z][y] = NoDraw;
		
		if (target.map[x][z][y+2] != 0 && neighboor.map[x][0][y+1] != 0 && target.map[x-1][z][y+1] != 0 && target.map[x+1][z][y+1] && target.map[x][z-1][y+1] != 0)
			target.status[x][z][y+1] = NoDraw;
			
		if (target.map[x][z][y-2] != 0 && neighboor.map[x][0][y-1] != 0 && target.map[x-1][z][y-1] != 0 && target.map[x+1][z][y-1] && target.map[x][z-1][y-1] != 0)
			target.status[x][z][y-1] = NoDraw;
	}
}

/*  огда будет свободное врем€ буду потихоньку делать флаг NoDraw дл€ углов чанка
void BlockCreatedAngleChunk(tChunk &target, tChunk &t1, tChunk &t2, tChunk &additionaly ,int x, int y, int z)
{
	if (x == 0 && z == 0)
	{
		if (t1.map[15][1][y] != 0 && t1.map[14][0][y] != 0 && t1.map[15][0][y+1] != 0 && t1.map[15][0][y-1] != 0 && additionaly.map[15][15][y] != 0)
			t1.status[15][0][y] = NoDraw;
		if (t2.map[0][14][y] != 0 && t2.map[1][15][y] != 0 && t2.map[0][15][y+1] != 0 && t2.map[0][15][y-1] != 0 && additionaly.map[15][15][y] != 0)
			t2.status[0][15][y] = NoDraw;
		if (t1.map[15][0][y+1] != 0 && t2.map[0][15][y+1] != 0 && target.map[0][0][y+2] != 0 && target.map[x+1][z][y+1] != 0 && target.map[x][z+1][y+1] != 0)
			target.status[x][z][y+1] = NoDraw;
		if (t1.map[15][0][y-1] != 0 && t2.map[0][15][y-1] != 0 && target.map[0][0][y-2] != 0 && target.map[x+1][z][y-1] != 0 && target.map[x][z+1][y-1] != 0)
			target.status[x][z][y-1] = NoDraw;
		if (t1.map[15][1][y] != 0 && target.map[0][2][y] != 0 && target.map[1][1][y] != 0 && target.map[0][1][y+1] != 0 && target.map[0][1][y-1] != 0)
			target.status[0][1][y] = NoDraw;
		if (t2.map[1][15][y] != 0 && target.map[2])
		
	}
	if (x == 15 && z == 0)
	{
		
	}
	if (x == 0 && z == 15)
	{
		
	}
	if (x == 15 && z == 15)
	{
		
	}
}
*/
typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        std::uint8_t B, G, R, A;
    };
} *PPixelInfo;

class Tga
{
private:
    std::vector<std::uint8_t> Pixels;
    bool ImageCompressed;
    std::uint32_t width, height, size, BitsPerPixel;

public:
    Tga(const char* FilePath);
    std::vector<std::uint8_t> GetPixels() {return this->Pixels;}
    std::uint32_t GetWidth() const {return this->width;}
    std::uint32_t GetHeight() const {return this->height;}
    bool HasAlphaChannel() {return BitsPerPixel == 32;}
};

Tga::Tga(const char* FilePath)
{
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()){throw MessageBox(NULL,"File not found!.","Texture TGA Load failed!",MB_OK | MB_ICONINFORMATION);}

    std::uint8_t Header[18] = {0};
    std::vector<std::uint8_t> ImageData;
    static std::uint8_t DeCompressed[12] = {0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    static std::uint8_t IsCompressed[12] = {0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    hFile.read(reinterpret_cast<char*>(&Header), sizeof(Header));

    if (!memcmp(DeCompressed, &Header, sizeof(DeCompressed)))
    {
        BitsPerPixel = Header[16];
        width  = Header[13] * 0x100 + Header[12];
        height = Header[15] * 0x100 + Header[14];
        size  = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
            throw MessageBox(NULL,"Invalid File Format. Required: 24 or 32 Bit Image.","Texture TGA Load failed!",MB_OK | MB_ICONINFORMATION);

        }

        ImageData.resize(size);
        ImageCompressed = false;
        hFile.read(reinterpret_cast<char*>(ImageData.data()), size);
    }
    else if (!memcmp(IsCompressed, &Header, sizeof(IsCompressed)))
    {
        BitsPerPixel = Header[16];
        width  = Header[13] * 0x100 + Header[12];
        height = Header[15] * 0x100 + Header[14];
        size  = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
          	throw MessageBox(NULL,"Invalid File Format. Required: 24 or 32 Bit Image.","Texture TGA Load failed!",MB_OK | MB_ICONINFORMATION);
        }

        PixelInfo Pixel = {0};
        int CurrentByte = 0;
        std::size_t CurrentPixel = 0;
        ImageCompressed = true;
        std::uint8_t ChunkHeader = {0};
        int BytesPerPixel = (BitsPerPixel / 8);
        ImageData.resize(width * height * sizeof(PixelInfo));

        do
        {
            hFile.read(reinterpret_cast<char*>(&ChunkHeader), sizeof(ChunkHeader));

            if(ChunkHeader < 128)
            {
                ++ChunkHeader;
                for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
                }
            }
            else
            {
                ChunkHeader -= 127;
                hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                for(int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24) ImageData[CurrentByte++] = Pixel.A;
                }
            }
        } while(CurrentPixel < (width * height));
    }
    else
    {
        hFile.close();
       	MessageBox(NULL,"Invalid File Format. Required: 24 or 32 Bit Image.","Texture TGA Load failed!",MB_OK | MB_ICONINFORMATION);
    }

    hFile.close();
    this->Pixels = ImageData;
}


