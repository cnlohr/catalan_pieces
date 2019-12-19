#include <stdio.h>
#include <math.h>

#define TAU (6.2831853)
#define EDGES 5
#define ETCH "stroke=\"#ff0000\" fill=\"none\" stroke-width=\".1mm\""
#define CUT  "stroke=\"#0000ff\" fill=\"none\" stroke-width=\".1mm\""
#define REFINEEDGE 3.5
#define DOTSIZE 1.5

//CNLohr's Super Basic SVG Toolkit.
int inpath = 0;
int started = 0;
const char * lastcolor = "black";
float centerx, centery;
void StartSVG( float width, float height ) { printf( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" ); printf( "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%fmm\" height=\"%fmm\" x=\"0\" y=\"0\" viewBox=\"0 0 %f %f\">\n", width, height, width, height ); }
void PathClose() { if( !inpath ) return; printf( "Z\" />\n" ); inpath = 0; }
void PathStart( const char * props ) { if( inpath ) PathClose(); lastcolor = props; printf( "<path %s d=\"", props ); inpath = 1; started = 0; }
void PathM( float x, float y ) { if( !inpath ) PathStart(lastcolor); printf( "M%f %f ", x+centerx, y+centery ); started = 1; }
void PathL( float x, float y ) { if( !inpath ) PathStart(lastcolor); printf( "%c%f %f ", started?'L':'M', x+centerx, y+centery ); started = 1; }
void PathQ( float xc, float yc, float x, float y ) { if( !inpath ) PathStart(lastcolor); printf( "Q%f %f, %f %f ", xc+centerx, yc+centery, x+centerx, y+centery ); started = 1; }
void PathAS( float r, float x, float y, int laf, int sf ) { printf( "A%f %f 0 %d %d %f %f ", r, r, laf, sf, x+centerx, y+centery ); }
void Circle( const char * props, float x, float y, float r ) { if( inpath ) PathClose(); printf( "<circle  cx=\"%f\" cy=\"%f\" r=\"%f\" %s />\n", x+centerx, y+centery, r, props ); }
void EndSVG() { PathClose(); printf( "</svg>" ); }

void Scale2d( float * out, float * a, float scale ) { out[0] = a[0] * scale; out[1] = a[1] * scale; }
void Sub2d( float * out, float * a, float * b ) { out[0] = a[0] - b[0]; out[1] = a[1] - b[1]; }
void Add2d( float * out, float * a, float * b ) { out[0] = a[0] + b[0]; out[1] = a[1] + b[1]; }
void Normalize2d( float * out, float * in ) { float mag = 1./sqrt( in[0] * in[0] + in[1] * in[1] ); Scale2d( out, in, mag ); }
void Normal2d( float * out, float * in ) { out[0] = -in[1]; out[1] = in[0]; }

//NOTE: This expects a wrapped-around coordset, allowing it to freely index off the end.
void EncompassSet( float * coordset, int numpoints )
{
	//Start off with edge going normal from the first two coords.
	int i;
	for( i = 0; i < numpoints; i++ )
	{
		float deltaedge1[2];
		float deltaedge2[2];
		Sub2d( deltaedge1, coordset+2, coordset+0 );
		Sub2d( deltaedge2, coordset+4, coordset+2 );
		float normedge1[2];
		float normedge2[2];
		Normal2d( normedge1, deltaedge1 );
		Normal2d( normedge2, deltaedge2 );
		Normalize2d( normedge1, normedge1 ); //In unitless
		Normalize2d( normedge2, normedge2 ); //In unitless
		Scale2d( normedge1, normedge1, REFINEEDGE ); //In mm
		Scale2d( normedge2, normedge2, REFINEEDGE ); //In mm
		float p1[2];
		float p2[2];
		float p3[2];
		Add2d( p1, coordset+0, normedge1 );
		Add2d( p2, coordset+2, normedge1 );
		Add2d( p3, coordset+2, normedge2 );
		if( i == 0 ) PathM( p1[0], p1[1] );
		PathL( p2[0], p2[1] );
		PathAS( REFINEEDGE, p3[0], p3[1], 0, 0 );
		//Now, the wrap-around.
		coordset+=2;
	}
}

float circlecoords[EDGES*2];

void EncompassCircles( char ** coords )
{
	float coordset[EDGES*2+4]; //Worst-case scenario.
	int i;
	int c = 0;
	do
	{
		PathStart( ETCH );
		for( i = 0; i < EDGES; i++ )
		{
			c = *((*coords)++);
			if( !c ) break;
			if( c == '/' || c == ',' ) break;
			c-='0';
			c%=EDGES;
			coordset[i*2+0] = circlecoords[c*2+0];
			coordset[i*2+1] = circlecoords[c*2+1];
		}
		//Extend past end to simplify logic in EncompassSet.
		coordset[i*2+0] = coordset[0];
		coordset[i*2+1] = coordset[1];
		coordset[i*2+2] = coordset[2];
		coordset[i*2+3] = coordset[3];
		EncompassSet( coordset, i );
		PathClose();
	} while( c == '/' );
}


int main()
{
	float width = 270;
	float height = 270;
	StartSVG( width, height );
	
	int i;
	for( i = 0; i < 42; i++ )
	{
		PathStart( CUT );
		centerx = (i % 7)*33+16;
		centery = (i / 7)*33+16;
		int edge = 0;
		int corners = EDGES;
		float rmain = 18;
		float taudiv = TAU/corners;
		float tauoffset = .16;
		
		for( edge = 0; edge < corners; edge++ )
		{
			float edginess = 0.2;
			float ang1 = (edge+edginess)*taudiv+tauoffset;
			float ang2 = (edge+(1.-edginess))*taudiv+tauoffset;
			float ang3 = (edge+1)*taudiv+tauoffset;
			float ang4 = (edge+1+edginess)*taudiv+tauoffset;
			float r2 = rmain;
			float r1 = r2*(sqrt(1-(sin(edginess*taudiv)))); //Not perfect but pretty good.
			PathL( sin(ang1)*r1, cos(ang1)*r1 );
			PathL( sin(ang2)*r1, cos(ang2)*r1 );
			PathQ( sin(ang3)*r2, cos(ang3)*r2, sin(ang4)*r1, cos(ang4)*r1 );
		}
		PathClose();
		for( edge = 0; edge < corners; edge++ )
		{
			float ang1 = (edge)*taudiv+tauoffset;
			float r2 = rmain*.6;
			Circle( edge?ETCH:CUT, circlecoords[edge*2+0] = sin(ang1)*r2, circlecoords[edge*2+1] = cos(ang1)*r2, DOTSIZE );
		}
	}
	const char * catalans = "\
,\
01,12,23,34,45,\
02,13,24,30,41,\
013,124,230,341,402,\
012,123,234,340,401,\
01/23,12/34,23/40,34/01,40/12,\
01/24,12/30,23/41,34/02,40/13,\
01/234,12/340,23/401,34/012,40/123,\
0123,1234,2340,3401,4012,\
01234,";
	i = 0;
	while( *catalans )
	{
		PathStart( ETCH );
		centerx = (i % 7)*33+16;
		centery = (i / 7)*33+16;
		EncompassCircles( &catalans );
		i++;
	}
	EndSVG();
}
