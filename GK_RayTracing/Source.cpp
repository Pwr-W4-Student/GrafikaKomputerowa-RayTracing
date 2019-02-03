#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <glut.h>
#include <gl/gl.h>
#include <fstream>
#include <string>

using namespace std;

typedef float vec[3];

#define MAX 2

int im_size_x = 0;
int im_size_y = 0;
int step = 0;

float viewport_size = 18.0;

float light_position[5][3];
float light_specular[5][3];
float light_diffuse[5][3];
float light_ambient[5][3];

float sphere_radius[9];
float sphere_xyz[9][3];
float sphere_specular[9][3];
float sphere_diffuse[9][3];
float sphere_ambient[9][3];
float sphere_specularhininess[9];

float global_a[3];

vec starting_punkt;
vec starting_directions = { 0.0, 0.0, -1.0 };

GLubyte pixel[1][1][3];
float background_color[3];

enum insersection
{
	LIGHT,
	SPHERE,
	NO_INTERSECTION
};


struct punktTabsphere {
	vec tab;
	int sphere;
	int light;
	//int status;
	insersection status;
};

struct pixelTab {
	GLubyte pixels[1][1][3];
};

struct vecT {
	vec tab;
};

/*************************************************************************************/
// Funkcja oblicza iloczyn skalarny wektorów.
/*************************************************************************************/
float vector_length(vecT vec)
{
	return (vec.tab[0] * vec.tab[0] + vec.tab[1] * vec.tab[1] + vec.tab[2] * vec.tab[2]);
}

float dotProduct(vec p1, vec p2)
{
	return (p1[0] * p2[0] + p1[1] * p2[1] + p1[2] * p2[2]);
}

//vecT Normalization(vec &q)
void Normalization(vec &q)
{
	vecT result;
	result.tab[0] = 0.0;
	result.tab[1] = 0.0;
	result.tab[2] = 0.0;
	float d = 0.0;
	int i;

	for (i = 0; i<3; i++)
		d += q[i] * q[i];

	d = sqrt(d);

	if (d > 0.0)
		for (i = 0; i<3; i++)
			//result.tab[i] = q[i] / (d*1.0);
			q[i] = q[i] / (d*1.0);

	//return result;
}

/*Funkcja do wyliczania sk³adowych wektora normalenego do powierzchni obiekt w punkcie q*/
vecT Normal(vec q, int sphere)
{
	vecT wynik;

	for (int i = 0; i < 3; i++)
	{
		wynik.tab[i] = (q[i] - sphere_xyz[sphere][i]) / sphere_radius[sphere];
	}

	return wynik;
}

/*Funkcja obliczaj¹ca kierunek odbicia promienia w pukcie q*/
vecT Reflect(vec startpunkt, vec q_sphere, vec normal)
{
	vecT result;
	vec ray;


	// oblicz wektor obiekt-obserwator
	ray[0] = startpunkt[0] - q_sphere[0];
	ray[1] = startpunkt[1] - q_sphere[1];
	ray[2] = startpunkt[2] - q_sphere[2];

	Normalization(ray);

	float n_dot_v;
	n_dot_v = dotProduct(ray, normal);

	//wyznacz promien odbity
	result.tab[0] = 2 * (n_dot_v)* normal[0] - ray[0];
	result.tab[1] = 2 * (n_dot_v)* normal[1] - ray[1];
	result.tab[2] = 2 * (n_dot_v)* normal[2] - ray[2];

	
	//jezeli wektor nie jest znormalizowany
	if (vector_length(result) > 1.0)
	{
		//return Normalization(result.tab);
		Normalization(result.tab);
		return result;
	}
	else
	{
		return result;
	}
}


/*Funkcja Intersect wyznacza wspó³rzêdne q punktu przeciêcia z najbli¿szym obiektem sceny,
//znajdujacym sie na drodze œledzonego promienia*/
punktTabsphere Intersect(vec punktStart, vec direct)
{
	punktTabsphere result;
	result.tab[0] = 0.0;
	result.tab[1] = 0.0;
	result.tab[2] = 0.0;
	result.sphere = 0;
	result.light = 0;
	result.status = NO_INTERSECTION;

	int status = 0;
	//int lights = 0;

	float a, b, c, delta, r;

	for (int lights = 0; lights < 6; lights++)
	{
		float x, y, z;
		x = light_position[lights][0] - punktStart[0];
		y = light_position[lights][1] - punktStart[1];
		z = light_position[lights][2] - punktStart[2];

		if ((x / direct[0]) == (y / direct[1]) && (y / direct[1]) == (z / direct[2]))
		{
			result.tab[0] = light_position[lights][0];
			result.tab[1] = light_position[lights][1];
			result.tab[2] = light_position[lights][2];
			result.status = LIGHT;
			return result;
		}
	}

	for (int sphere = 0; sphere < 9; sphere++)
	{
		a = direct[0] * direct[0] + direct[1] * direct[1] + direct[2] * direct[2];

		b = 2 * ((punktStart[0] - sphere_xyz[sphere][0])*direct[0] + (punktStart[1]
			- sphere_xyz[sphere][1])*direct[1] + (punktStart[2] - sphere_xyz[sphere][2])*direct[2]);

		c = (punktStart[0] * punktStart[0] + punktStart[1] * punktStart[1] + punktStart[2] * punktStart[2])
			+ (sphere_xyz[sphere][0] * sphere_xyz[sphere][0] + sphere_xyz[sphere][1] * sphere_xyz[sphere][1]
				+ sphere_xyz[sphere][2] * sphere_xyz[sphere][2])
			- 2 * (punktStart[0] * sphere_xyz[sphere][0] + punktStart[1] * sphere_xyz[sphere][1] + punktStart[2] * sphere_xyz[sphere][2])
			- sphere_radius[sphere] * sphere_radius[sphere];

		delta = b*b - 4 * a*c;

		if (delta >= 0)
		{
			r = (-b - sqrt(delta)) / (2 * a);

			if (r > 0)
			{
				result.tab[0] = punktStart[0] + r*direct[0];
				result.tab[1] = punktStart[1] + r*direct[1];
				result.tab[2] = punktStart[2] + r*direct[2];
				result.sphere = sphere;
				result.status = SPHERE;
				break;
			}
		}
	}

	return result;
}


//q - starting_punkt
//n - znormalizowany(q)
//d - starting_directions

/*************************************************************************************/
// Funkcja oblicza oœwietlenie punktu na powierzchni sfery u¿ywaj¹c modelu Phonga.
/*************************************************************************************/
vecT Phong(vec q, vec n, vec dest, int nr_sphere)
{
	vecT color;
	color.tab[0] = 0.0;
	color.tab[1] = 0.0;
	color.tab[2] = 0.0;

	vec lightVector;                    // wektor wskazuj¹cy Ÿróde³
	vec reflectionVector;            // wektor kierunku odbicia œwiat³a
	vec viewer_v = { 0.0, 0.0, 1.0 };   // wektor kierunku obserwacji
	float doPLight, doPReflect;                // zmienne pomocnicze

	float a, b, c, wspl;
	a = 1.0;
	b = 0.1;
	c = 0.01;
	wspl = 1 / (a + b + c);


	for (int k = 0; k < 5; k++) // liczba zrodel swiatla
	{
		//wektor swiatla i jego dlugosc
		lightVector[0] = light_position[k][0] - q[0];
		lightVector[1] = light_position[k][1] - q[1];
		lightVector[2] = light_position[k][2] - q[2];

		vecT light_vec_loc;
		Normalization(lightVector);

		doPLight = dotProduct(lightVector, n);

		reflectionVector[0] = 2 * (doPLight)*n[0] - lightVector[0];
		reflectionVector[1] = 2 * (doPLight)*n[1] - lightVector[1];
		reflectionVector[2] = 2 * (doPLight)*n[2] - lightVector[2];

		vecT reflection_vector_loc;
		Normalization(reflectionVector);

		doPReflect = dotProduct(reflectionVector, viewer_v);

		if (doPReflect < 0)                  // obserwator nie widzi oœwietlanego punktu
			doPReflect = 0;

		// sprawdzenie czy vec na powierzchni sfery jest oœwietlany przez Ÿród³o

		if (doPLight > 0)    // vec jest oœwietlany,
		{                   // oœwietlenie wyliczane jest ze wzorów dla modelu Phonga

			color.tab[0] += wspl*(sphere_diffuse[nr_sphere][0] * light_diffuse[k][0] * doPLight + sphere_specular[nr_sphere][0] * light_specular[k][0] * pow(double(doPReflect), double(sphere_specularhininess[nr_sphere]))) + sphere_ambient[nr_sphere][0] * light_ambient[k][0] + sphere_ambient[nr_sphere][0] * global_a[0];

			color.tab[1] += wspl*(sphere_diffuse[nr_sphere][1] * light_diffuse[k][1] * doPLight + sphere_specular[nr_sphere][1] * light_specular[k][1] * pow(double(doPReflect), double(sphere_specularhininess[nr_sphere]))) + sphere_ambient[nr_sphere][1] * light_ambient[k][1] + sphere_ambient[nr_sphere][1] * global_a[1];

			color.tab[2] += wspl*(sphere_diffuse[nr_sphere][2] * light_diffuse[k][2] * doPLight + sphere_specular[nr_sphere][2] * light_specular[k][2] * pow(double(doPReflect), double(sphere_specularhininess[nr_sphere]))) + sphere_ambient[nr_sphere][2] * light_ambient[k][2] + sphere_ambient[nr_sphere][2] * global_a[2];
		}
		else                // vec nie jest oœwietlany  
		{                   // uwzglêdniane jest tylko œwiat³o rozproszone

			color.tab[0] += sphere_ambient[nr_sphere][0] *global_a[0];
			color.tab[1] += sphere_ambient[nr_sphere][1] *global_a[1];
			color.tab[2] += sphere_ambient[nr_sphere][2] *global_a[2];

		}

	}

	return color;
}

//p - starting_punkt
//d - starting_directions
/*************************************************************************************/
// Funkcja oblicza vec przeciêcia promienia i powierzchni sfery
// Argument p jest punktem pocz¹tkowym promienia a d wektorem opisuj¹cym
// kierunek biegu promienia
/*************************************************************************************/
vecT Trace(vec p, vec d, int step)
{
	vec v_normal; // wektor normalny do powierzchni obiektu
	vec normal; // wektor znormalizowany
	vec reflect; // wektor odbity od powierzchni

	insersection status; //status trafienie promienia
	int nr_light; // numer zrodla swiatla
	int nr_sphere; // numer sfery

	vecT color;
	color.tab[0] = 0.0;
	color.tab[1] = 0.0;
	color.tab[2] = 0.0;

	vecT reflected;
	reflected.tab[0] = 0.0;
	reflected.tab[1] = 0.0;
	reflected.tab[2] = 0.0;

	if (step > MAX)
	{
		return color;
	}

	punktTabsphere q_loc;
	q_loc = Intersect(p, d);
	v_normal[0] = q_loc.tab[0];
	v_normal[1] = q_loc.tab[1];
	v_normal[2] = q_loc.tab[2];
	nr_sphere = q_loc.sphere;
	nr_light = q_loc.light;
	status = q_loc.status;

	//zrodlo swiatla
	if (status == LIGHT)
	{
		color.tab[0] += light_specular[nr_light][0];
		color.tab[1] += light_specular[nr_light][1];
		color.tab[2] += light_specular[nr_light][2];
		return color;
	}

	//nic nie zostalo trafione
	if (status == NO_INTERSECTION)
	{
		color.tab[0] += background_color[0];
		color.tab[1] += background_color[1];
		color.tab[2] += background_color[2];
		return color;
	}

	//wyliczenie wektora znormalizowanego do powierzchni
	vecT n_loc;
	n_loc = Normal(v_normal, nr_sphere);
	normal[0] = n_loc.tab[0];
	normal[1] = n_loc.tab[1];
	normal[2] = n_loc.tab[2];

	//wyliczenie promienia odbitego
	vecT r_loc;
	r_loc = Reflect(p, v_normal, normal);
	reflect[0] = r_loc.tab[0];
	reflect[1] = r_loc.tab[1];
	reflect[2] = r_loc.tab[2];

	//obliczenie koloru punktu
	// q - vec na sferze,
	// n - wektor znormalizowany(q),
	// d - wektor startowy
	color = Phong(v_normal, normal, d, nr_sphere);

	//obliczenie kolorow nastepnych punktow
	reflected = Trace(v_normal, reflect, step + 1);


	color.tab[0] *= 0.5;
	color.tab[1] *= 0.5;
	color.tab[2] *= 0.5;

	color.tab[0] += 0.6* reflected.tab[0];
	color.tab[1] += 0.6 * reflected.tab[1];
	color.tab[2] += 0.6 * reflected.tab[2];


	return color;
};

/*************************************************************************************/
// Funkcja rysuj¹ca obraz oœwietlonej sceny.
/*************************************************************************************/
void Display(void)
{
	int  x, y;           // po³o¿enie rysowanego piksela "ca³kowitoliczbowa"
	float x_fl, y_fl;    // po³o¿enie rysowanego piksela "zmiennoprzecinkowa"
	int im_size_x_2;       // po³owa rozmiaru obrazu w pikselach
	int im_size_y_2;

	im_size_x_2 = im_size_x / 2;    // obliczenie po³owy rozmiaru obrazu w pikselach
	im_size_y_2 = im_size_y / 2;
	glClear(GL_COLOR_BUFFER_BIT);

	glFlush();

	// rysowanie pikseli od lewego górnego naro¿nika do prawego dolnego naro¿nika

	for (y = im_size_y_2; y > -im_size_y_2; y--)
	{

		for (x = -im_size_x_2; x < im_size_x_2; x++)
		{

			x_fl = (float)x / (im_size_x / viewport_size);
			y_fl = (float)y / (im_size_y / viewport_size);

			starting_punkt[0] = x_fl;
			starting_punkt[1] = y_fl;
			starting_punkt[2] = viewport_size;

			vecT color;
			color = Trace(starting_punkt, starting_directions, step);

			if (color.tab[0] > 1)
				pixel[0][0][0] = 255;
			else
				pixel[0][0][0] = color.tab[0] * 255;

			if (color.tab[1] > 1)
				pixel[0][0][1] = 255;
			else
				pixel[0][0][1] = color.tab[1] * 255;

			if (color.tab[2] > 1)
				pixel[0][0][2] = 255;
			else
				pixel[0][0][2] = color.tab[2] * 255;

			glRasterPos3f(x_fl, y_fl, 0);

			// inkrementacja pozycji rastrowej dla rysowania piksela

			glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

			glFlush();
		}
	}

}


void ReadFile(void)  //odczytywanie danych z pliku
{
	fstream file;
	file.open("scene.txt");
	string tmp;
	if (file.is_open() != true)
	{
		cout << "Nie udalo wczytac sie pliku";
	}
	else
	{
		file >> tmp;

		file >> im_size_x;
		file >> im_size_y;

		file >> tmp;
		for (int i = 0; i < 3; i++) file >> background_color[i];

		file >> tmp;
		for (int i = 0; i < 3; i++) file >> global_a[i];
		int i_sp = 0;
		int i_so = 0;

		for (int i = 0; i < 9; i++)
		{
			file >> tmp;
			file >> sphere_radius[i];
			file >> sphere_xyz[i][0];
			file >> sphere_xyz[i][1];
			file >> sphere_xyz[i][2];

			file >> sphere_specular[i][0];
			file >> sphere_specular[i][1];
			file >> sphere_specular[i][2];

			file >> sphere_diffuse[i][0];
			file >> sphere_diffuse[i][1];
			file >> sphere_diffuse[i][2];

			file >> sphere_ambient[i][0];
			file >> sphere_ambient[i][1];
			file >> sphere_ambient[i][2];
			file >> sphere_specularhininess[i];
		}

		for (int i = 0; i < 5; i++)
		{
			file >> tmp;
			file >> light_position[i][0];
			file >> light_position[i][1];
			file >> light_position[i][2];
			file >> light_specular[i][0];
			file >> light_specular[i][1];
			file >> light_specular[i][2];
			file >> light_diffuse[i][0];
			file >> light_diffuse[i][1];
			file >> light_diffuse[i][2];
			file >> light_ambient[i][0];
			file >> light_ambient[i][1];
			file >> light_ambient[i][2];
		}

		file.close();
	}

}

/*************************************************************************************/
// Funkcja inicjalizuj¹ca definiuj¹ca sposób rzutowania.
/*************************************************************************************/
void Myinit(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-viewport_size / 2, viewport_size / 2, -viewport_size / 2, viewport_size / 2, -viewport_size / 2, viewport_size / 2);
	glMatrixMode(GL_MODELVIEW);
}

/*************************************************************************************/
// Funkcja g³ówna.
/*************************************************************************************/

int main(void)
{
	ReadFile();
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(im_size_x, im_size_y);
	glutCreateWindow("Ray Tracing");
	Myinit();
	glutDisplayFunc(Display);
	glutMainLoop();
}