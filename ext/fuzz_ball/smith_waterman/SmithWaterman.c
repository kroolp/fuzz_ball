// Include the Ruby headers and goodies
#include <ruby.h>
#include <SmithWaterman.h>

// The initialization method for this module
void Init_smith_waterman() {
	FuzzBall      = rb_define_module("FuzzBall");
	SmithWaterman = rb_define_class_under(FuzzBall, "SmithWaterman", rb_cObject);

	rb_define_method(SmithWaterman, "initialize", method_initialize, 2);
	rb_define_attr(SmithWaterman, "alignment", 1, 0);
	rb_define_attr(SmithWaterman, "score", 1, 0);
}

/* method_initialize
 *
 * We use the Smith-Waterman algorithm to align each candidate string with the
 * needle string and to see how well the two fit. The Smith-Waterman algorithm
 * is a dynamic programming algorith that keeps track of different alignments
 * between two strings using a matrix. The best alignment is determined using
 * a recursive search through the alignment matrix. For more information, see:
 *
 * http://en.wikipedia.org/wiki/Smith%E2%80%93Waterman_algorithm
 */
VALUE method_initialize(VALUE self, VALUE needle, VALUE candidate) {
  int i, j, i_max, j_max;
  int n_needle    = (int) RARRAY_LEN(needle);
  int n_candidate = (int) RARRAY_LEN(candidate);
  int *c_needle, *c_candidate;

  double max_score;
  double **mat;

  VALUE alignment = rb_ary_new();

  alloc_vars(&mat, &c_needle, &c_candidate, n_needle, n_candidate);

  // Copy the needle / candidate strings from their ruby versions
  // into plain old C-integer arrays.
  for (i=0; i<n_needle; i++) {
    c_needle[i] = NUM2INT( RARRAY_PTR(needle)[i] );
  }

  for (i=0; i<n_candidate; i++) {
    c_candidate[i] = NUM2INT( RARRAY_PTR(candidate)[i] );
  }

  assign_cells(mat, c_needle, c_candidate, n_needle, n_candidate, &i_max, &j_max, &max_score);

  rb_iv_set(self, "@alignment", alignment);
  rb_iv_set(self, "@score", DBL2NUM(max_score));

  free_vars(&mat, &c_needle, &c_candidate, n_needle, n_candidate);

	return self;
}

// A few convenience methods for determining the max of 2, 3, and 4 doubles
double max(double a, double b) {
  return ( (a > b) ? a : b );
}

double max3(double a, double b, double c) {
  return max(a, max(b, c));
}

double max4(double a, double b, double c, double d) {
  return max(a, max3(b, c, d));
}

/* alloc_vars
 *
 * A simple function that allocates memory for the alignment matrix, as well
 * as the arrays that store the characters of the needle and candidate strings
 * given their lengths.
 */
void alloc_vars(
    double ***mat,
    int **needle,
    int **candidate,
    int n_needle,
    int n_candidate
) {
  int i;

  *mat = malloc(n_needle * sizeof(double *));
  for (i=0; i<n_needle; i++) {
    *((*mat) + i) = malloc(n_candidate * sizeof(double)); // ptr arithmetic FTW
  }

  *needle    = malloc(n_needle * sizeof(int));
  *candidate = malloc(n_candidate * sizeof(int));
}

/* free_vars
 *
 * The method that frees memory associated with the alignment matrix, and the
 * needle and candidate strings.
 */
void free_vars(
    double ***mat,
    int **needle,
    int **candidate,
    int n_needle,
    int n_candidate
) {

  int i;

  free(*needle);    *needle = NULL;
  free(*candidate); *candidate = NULL;

  for (i=0; i<n_needle; i++) {
    free( (*mat)[i] );
  }
  free( *mat );
  *mat = NULL;
}

/* assign_cells
 *
 * Called within the smith_waterman loop; this is the function that assigns
 * each cell of the alignment matrix; the value of each cell represents the
 * score of that given alignment, up to that point, taking into account any
 * deletions, additions, etc.. that result in that alignment. For two strings
 * of length m, n the alignment matrix is m x n large. As the values are
 * assigned, we keep track of the cell with the higest score. At the end of
 * the assignment, we start at this highest-scoring cell and recursively walk
 * backwards through the cells, maximizing the score at each step. This
 * becomes the highest scoring alignment
 */
void assign_cells(
    double **mat,
    int *needle,
    int *candidate,
    int n_needle,
    int n_candidate,
    int *i_max,
    int *j_max,
    double *max_score
) {
  int i, j;
  double score, value;

  for (i=0; i<n_needle; i++) {
    for (j=0; j<n_candidate; j++) {
      mat[i][j] = 0.0;
    }
  }

  *max_score = -10000.0;
  for (i=1; i<n_needle; i++) {
    for (j=1; j<n_candidate; j++) {
      if (needle[i-1] == candidate[j-1]) {
        score = SCORE_MATCH;
      } else {
        score = SCORE_MISS;
      }

      value = max4(0.0, mat[i-1][j-1] + score, mat[i-1][j] + SCORE_DELETE, mat[i][j-1] + SCORE_INSERT);
      mat[i][j] = value;

      if (value > *max_score) {
        *max_score = value;
        *i_max     = i;
        *j_max     = j;
      }
    }
  }
}
