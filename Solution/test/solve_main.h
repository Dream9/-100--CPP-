#ifndef _SOLUTION_SOLVE_MAIN_H_
#define _SOLUTION_SOLVE_MAIN_H_

#include"Solution/solution.h"


#define NO_063

#ifdef NO_001
#include"Solution/001-010/channel_swap.h"
#endif

#ifdef NO_002
#include"Solution/001-010/grayscale.h"
#endif

#ifdef NO_003
#include"Solution/001-010/binarization.h"
#endif

#ifdef NO_004
#include"Solution/001-010/otsu.h"
#endif

#ifdef NO_005
#include"Solution/001-010/reverse_hue.h"
#endif

#ifdef NO_006
#include"Solution/001-010/reduce_color.h"
#endif

#ifdef NO_007
#include"Solution/001-010/mean_pooling.h"
#endif

#ifdef NO_008
#include"Solution/001-010/max_pooling.h"
#endif

#ifdef NO_009
#include"Solution/001-010/gaussian_filter.h"
#endif

#ifdef NO_010
#include"Solution/001-010/median_blur.h"
#endif

#ifdef NO_011
#include"Solution/011-020/mean_blur.h"
#endif

#ifdef NO_012
#include"Solution/011-020/motion_blur.h"
#endif

#ifdef NO_013
#include"Solution/011-020/max_min_filter.h"
#endif

#ifdef NO_014
#include"Solution/011-020/differential_filter.h"
#endif

#ifdef NO_015
#include"Solution/011-020/sobel_operator.h"
#endif

#ifdef NO_016
#include"Solution/011-020/prewitt_operator.h"
#endif

#ifdef NO_017
#include"Solution/011-020/laplacian.h"
#endif

#ifdef NO_018
#include"Solution/011-020/emboss_filter.h"
#endif

#ifdef NO_019
#include"Solution/011-020/LoG_filter.h"
#endif

#ifdef NO_020
#include"Solution/011-020/histogram.h"
#endif

#ifdef NO_021
#include"Solution/021-030/normalization.h"
#endif

#ifdef NO_022
#include"Solution/021-030/histogram_transformation.h"
#endif

#ifdef NO_023
#include"Solution/021-030/equalize_histogram.h"
#endif

#ifdef NO_024
#include"Solution/021-030/gamma_correction.h"
#endif

#ifdef NO_025
#include"Solution/021-030/nearest_neighbor_interpolation.h"
#endif

#ifdef NO_026
#include"Solution/021-030/bilinear_interpolation.h"
#endif

#ifdef NO_027
#include"Solution/021-030/bicubic_interpolation.h"
#endif


#if (defined NO_028) || (defined NO_029) || (defined NO_030) || (defined NO_031)
#include"Solution/021-030/affine_transformation.h"
#endif

#ifdef NO_032
#include"Solution/031-040/fourier.h"
#endif

#ifdef NO_033
#include"Solution/031-040/frequency_domain_filter_low_pass.h"
#endif

#ifdef NO_034
#include"Solution/031-040/frequency_domain_filter_high_pass.h"
#endif

#ifdef NO_035
#include"Solution/031-040/frequency_domain_filter_band.h"
#include"Solution/031-040/notch_filter.h"
#endif

#if (defined NO_041) || (defined NO_042) || (defined NO_043)
#include"Solution/041-050/canny.h"
#endif

#if (defined NO_044) || (defined NO_045) || (defined NO_046)
#include"Solution/041-050/hough_lines.h"
#endif

#if (defined NO_047) || (defined NO_048) || (defined NO_049) || (defined NO_050) || \
    (defined NO_051) || (defined NO_052) || (defined NO_053)
#include"Solution/041-050/morphology.h"
#endif

#if (defined NO_058) || (defined NO_059)
#include"Solution/051-060/connected_component_label.h"
#endif

#if (defined NO_061) || (defined NO_062)
#include"Solution/061-070/connect_number.h"
#endif

#if defined NO_063
#include"Solution/061-070/thin.h"
#endif



#endif // _SOLUTION_SOLVE_MAIN_H_
