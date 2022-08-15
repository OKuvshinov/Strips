#include "pch.h"
#include "IMMDraw.h"

CPen blackpan(PS_SOLID, 1, RGB(0, 0, 0));
CPen bluepan(PS_SOLID, 1, RGB(0, 0, 255));
CPen redpan(PS_SOLID, 1, RGB(255, 0, 0));

int center_window_x = 0;
int center_window_y = 0;
int scale_param = 1;
int scale_helper_param = 1;

void draw_object(vector<Paths> object, CString tool, CString color, CString regime, CPaintDC dc)
{
	if (color == "Black")
	{
		dc.SelectObject(blackpan);
	}

	//StructureForDraw = make_structure_for_draw(StructureForDraw, object);

	//for (int i = 0; i < object[0][i].size(); i++) {
	//	dc.Polygon(StructureForDraw[i], object[0][i].size());
	//}
}

POINT * make_structure_for_draw(POINT * structure, vector<Path> object)
{
	structure = new POINT[object[0].size()];
	for (int i = 0; i < object[0].size(); i++) {
		structure[i].x = center_window_x + (scale_param * object[0][i].X) / scale_helper_param;
		structure[i].y = center_window_y - (scale_param * object[0][i].Y) / scale_helper_param;
	}

	return structure;
}

POINT ** make_structure_for_draw(POINT ** structure, vector<Paths> object)
{
	delete[] structure;

	structure = new POINT*[object[0].size()];
	for (int i = 0; i < object[0].size(); i++) {
		structure[i] = new POINT[object[0][i].size()];
	}
	for (int i = 0; i < object[0].size(); i++) {
		for (int j = 0; j < object[0][i].size(); j++) {
			structure[i][j].x = center_window_x + (scale_param * object[0][i][j].X) / scale_helper_param;
			structure[i][j].y = center_window_y - (scale_param * object[0][i][j].Y) / scale_helper_param;
		}
	}

	return structure;
}

void set_drawing_param(int center_x, int center_y, int scale_value, int scale_helper_value)
{
	center_window_x = center_x;
	center_window_y = center_y;
	scale_param = scale_value;
	scale_helper_param = scale_helper_value;
}

vector<Paths> PolyFromDot(1);
vector<Path> NewDot(1);
double XForDotPoly = 0;
double YForDotPoly = 0;

bool ThisDotIsAlreadyChecked = false;

vector<Paths> make_polys_for_dots(vector<Path> paths, int num_of_path, vector<Path> check_dots)
{
	PolyFromDot[0].clear();

	for (int i = 0; i < paths[num_of_path].size(); i++) {

		// проверим, не сделали ли мы маааленький треугольник дл€ Ё“ќ… точки. Ёто ускорило процесс с 44 сек, до 33!!!
		for (int j = 0; j < check_dots[0].size(); j++) {
			if ((paths[num_of_path][i].X == check_dots[0][j].X) && (paths[num_of_path][i].Y == check_dots[0][j].Y)) {
				ThisDotIsAlreadyChecked = true;
				break;
			}
		}
		if (ThisDotIsAlreadyChecked == true) { // если считали, то пропускаем ее
			ThisDotIsAlreadyChecked = false;
			continue;
		}


		NewDot[0].clear();
		for (int j = 0; j < 3; j++) {
			XForDotPoly = floor(paths[num_of_path][i].X + (cos((90 - (120 * j))*PI / 180)) * (0.01 * scale_helper_param)); /////////////////
			YForDotPoly = floor(paths[num_of_path][i].Y - (sin((90 - (120 * j))*PI / 180)) * (0.01 * scale_helper_param)); /////////////////

			NewDot[0] << IntPoint(XForDotPoly, YForDotPoly);
		}
		PolyFromDot[0].push_back(NewDot[0]);
	}

	return PolyFromDot;
}

