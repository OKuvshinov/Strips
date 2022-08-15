#pragma once

#include "clipper.hpp"

using namespace ClipperLib;

const double PI = 3.141592;

void draw_object(vector<Paths> object, CString tool, CString color, CString regime, CPaintDC dc);

POINT *make_structure_for_draw(POINT *structure, vector<Path> object);

POINT **make_structure_for_draw(POINT **structure, vector<Paths> object);

void set_drawing_param(int center_x, int center_y, int scale_value, int scale_helper_value);

vector<Paths> make_polys_for_dots(vector<Path> paths, int num_of_path, vector<Path> check_dots);