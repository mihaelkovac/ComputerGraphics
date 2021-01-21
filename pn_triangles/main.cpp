#include "pn_triangle_app.h"

int main(int argc, const char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Must specify path to model as input argument!");
		exit(1);
	}
	/*const char* model = "D:/FER/diplomski/3.semestar/RG/labosi/lab3/Lab3/models/teddy.obj";*/
	const char* model = argv[1];
	PNTriangleApp app{};
	app.Run(1280, 720, model);
	app.Release();

	

	return 0;
}