#include "FPToolkit.c"
#include <assert.h>

// Hard-coded coefficients, scary!

const double a_coef[4] = { 1.0f/6, -1.0f/2,  1.0f/2, -1.0f/6 };
const double b_coef[4] = { 2.0f/3,     0.0,   -1.0f,  1.0f/2 };
const double c_coef[4] = { 1.0f/6,  1.0f/2,  1.0f/2, -1.0f/2 };
const double d_coef[4] = {   0.0f,    0.0f,    0.0f,  1.0f/6 };


/* Returns remainder of synthetic division. Accepts NULL argument for `quot`.
 * Copied from Homework 2.
 */
double syndiv(int degree, double num[], double denom, double quot[])
{
  if (degree <= 0)
    return num[0];

  if (quot) {
    quot[0] = syndiv(degree - 1, num + 1, denom, quot + 1);
    return num[0] + denom * quot[0];
  } else
    return num[0] + denom * syndiv(degree - 1, num + 1, denom, NULL);
}


double ** new_spline(int n_segments, int degree)
{ 
  double ** rows = malloc(sizeof(double*) * n_segments);
  
  for (int i = 0; i < n_segments; ++i)
    rows[i] = malloc(sizeof(double) * (degree + 1));
  
  return rows;
}


void del_spline(int n_segments, double **s)
{ 
  for (int i = 0; i < n_segments; ++i)
    free(s[i]);
  free(s);
}


void add_scaled_f(int deg, double *accum, const double *f, double c)
{
  for (int i = 0; i <= deg; i++)
    accum[i] += f[i] * c;
}


void cubic_seg(int i_seg, double *seg, int n_pts, double *pts)
{
  assert(i_seg + 3 < n_pts);

  const double *funcs[] = {a_coef, b_coef, c_coef, d_coef};

  for (int i = 0; i < 4; i++)
    seg[i] = 0; 

  for (int i = 0; i < 4; i++)
    add_scaled_f(3, seg, funcs[i], pts[i_seg + i]);
}


void cubic_spline(double **s, int n_pts, double *pts)
{
  assert(n_pts > 3);

  for (int i = 0; i < n_pts - 3; i++)
    cubic_seg(i, s[i], n_pts, pts);
}


void G_spline_seg(int degree, double *segx, double *segy)
{
  double dt = 1.0f/1024.0f;
  double x, y;
  double x_old = syndiv(degree, segx, 0, NULL);
  double y_old = syndiv(degree, segy, 0, NULL);

  G_rgb(1.0f, 0.0f, 0.0f); // red
  for (double t = 0; t <= 1.0f; t += dt) {
    x = syndiv(degree, segx, t, NULL);
    y = syndiv(degree, segy, t, NULL);
    G_line(x_old, y_old, x, y);
    x_old = x;
    y_old = y; 
  }
}


void G_cubic_spline(int n_segments, double **sx, double **sy)
{
  for (int i = 0; i < n_segments; i++) 
    G_spline_seg(3, sx[i], sy[i]);
}


void pts_to_cubic_spline(int n_pts, double *xpts, double *ypts)
{
  double **sx = new_spline(n_pts - 3, 3);
  double **sy = new_spline(n_pts - 3, 3);

  cubic_spline(sx, n_pts, xpts);
  cubic_spline(sy, n_pts, ypts);

  G_cubic_spline(n_pts - 3, sx, sy);

  del_spline(n_pts - 3, sx);
  del_spline(n_pts - 3, sy);
}


int main(int argc, char **argv) {
  if (argc != 2) {
    printf("%s: exactly 1 argument required\n", argv[0]);
    return 0;
  }

  int mode = argv[1][0] - '0';

  if ( (mode != 0 && mode != 1) || strlen(argv[1]) != 1 ) {
    printf("%s: %s is not 0 or 1\n", argv[0], argv[1]);
    return 0;
  }

  const int MAX_PTS = 100;
  double x[MAX_PTS];
  double y[MAX_PTS];
  double f[MAX_PTS];
  double stripwidth = 10;
  double boxx = 800;
  double boxy = 800;
  double swidth = boxx + stripwidth;
  double sheight = boxy;
  double radius = 2;
  int i = 0;
  double p[2] = {0, 0};
  int maxi = MAX_PTS;

  // BACKGROUND
  G_init_graphics (swidth,sheight) ;  // interactive graphics
  G_rgb (0.0, 0.0, 0.0) ; // black
  G_clear ();

  // DRAW STRIP BOUNDARY
  G_rgb (0.0, 0.0, 1.0) ; // blue
  G_line (boxx, 0, boxx, sheight-1);

  // GET POINTS
  if (mode == 1) {
    scanf("%d", &maxi);
  }

  for (i = 0; i < MAX_PTS && i < maxi; i++) {
    if (mode == 0) {
      G_wait_click(p);
      if (p[0] > boxx)
        break;
    } else {
      scanf("%lf %lf", &p[0], &p[1]);
    }

    x[i] = p[0];
    y[i] = p[1];

    // DRAW CIRCLES
    G_rgb(1.0, 0.0, 0.0) ; // red
    G_fill_circle(p[0], p[1], radius);
  }

  // DRAW PARABOLA OF BEST FIT
  // quad_reg(i, x, y, 0, boxx);

  // DRAW CUBIC SPLINE
  if (i > 3)
    pts_to_cubic_spline(i, x, y);
  else 
    printf("not enough points\n");

  int key ;
  key = G_wait_key() ; // pause so user can see results
  G_save_image_to_file("demo.xwd");
  return 0;
}
