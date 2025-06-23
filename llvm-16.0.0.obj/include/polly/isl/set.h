/*
 * Copyright 2008-2009 Katholieke Universiteit Leuven
 *
 * Use of this software is governed by the MIT license
 *
 * Written by Sven Verdoolaege, K.U.Leuven, Departement
 * Computerwetenschappen, Celestijnenlaan 200A, B-3001 Leuven, Belgium
 */

#ifndef ISL_SET_H
#define ISL_SET_H

#include <isl/id_type.h>
#include <isl/map_type.h>
#include <isl/aff_type.h>
#include <isl/list.h>
#include <isl/mat.h>
#include <isl/point.h>
#include <isl/local_space.h>
#include <isl/val_type.h>
#include <isl/stdint.h>
#include <isl/stride_info.h>
#include <isl/fixed_box.h>

#if defined(__cplusplus)
extern "C" {
#endif

isl_size isl_basic_set_n_dim(__isl_keep isl_basic_set *bset);
isl_size isl_basic_set_n_param(__isl_keep isl_basic_set *bset);
isl_size isl_basic_set_total_dim(__isl_keep const isl_basic_set *bset);
isl_size isl_basic_set_dim(__isl_keep isl_basic_set *bset,
				enum isl_dim_type type);

isl_size isl_set_n_dim(__isl_keep isl_set *set);
isl_size isl_set_n_param(__isl_keep isl_set *set);
__isl_export
isl_size isl_set_tuple_dim(__isl_keep isl_set *set);
isl_size isl_set_dim(__isl_keep isl_set *set, enum isl_dim_type type);

isl_ctx *isl_basic_set_get_ctx(__isl_keep isl_basic_set *bset);
isl_ctx *isl_set_get_ctx(__isl_keep isl_set *set);
__isl_give isl_space *isl_basic_set_get_space(__isl_keep isl_basic_set *bset);
__isl_export
__isl_give isl_space *isl_set_get_space(__isl_keep isl_set *set);
__isl_give isl_set *isl_set_reset_space(__isl_take isl_set *set,
	__isl_take isl_space *space);

__isl_give isl_aff *isl_basic_set_get_div(__isl_keep isl_basic_set *bset,
	int pos);

__isl_give isl_local_space *isl_basic_set_get_local_space(
	__isl_keep isl_basic_set *bset);

const char *isl_basic_set_get_tuple_name(__isl_keep isl_basic_set *bset);
isl_bool isl_set_has_tuple_name(__isl_keep isl_set *set);
const char *isl_set_get_tuple_name(__isl_keep isl_set *set);
__isl_give isl_basic_set *isl_basic_set_set_tuple_name(
	__isl_take isl_basic_set *set, const char *s);
__isl_give isl_set *isl_set_set_tuple_name(__isl_take isl_set *set,
	const char *s);
const char *isl_basic_set_get_dim_name(__isl_keep isl_basic_set *bset,
	enum isl_dim_type type, unsigned pos);
__isl_give isl_basic_set *isl_basic_set_set_dim_name(
	__isl_take isl_basic_set *bset,
	enum isl_dim_type type, unsigned pos, const char *s);
isl_bool isl_set_has_dim_name(__isl_keep isl_set *set,
	enum isl_dim_type type, unsigned pos);
const char *isl_set_get_dim_name(__isl_keep isl_set *set,
	enum isl_dim_type type, unsigned pos);
__isl_give isl_set *isl_set_set_dim_name(__isl_take isl_set *set,
	enum isl_dim_type type, unsigned pos, const char *s);

__isl_give isl_id *isl_basic_set_get_dim_id(__isl_keep isl_basic_set *bset,
	enum isl_dim_type type, unsigned pos);
__isl_give isl_basic_set *isl_basic_set_set_tuple_id(
	__isl_take isl_basic_set *bset, __isl_take isl_id *id);
__isl_give isl_set *isl_set_set_dim_id(__isl_take isl_set *set,
	enum isl_dim_type type, unsigned pos, __isl_take isl_id *id);
isl_bool isl_set_has_dim_id(__isl_keep isl_set *set,
	enum isl_dim_type type, unsigned pos);
__isl_give isl_id *isl_set_get_dim_id(__isl_keep isl_set *set,
	enum isl_dim_type type, unsigned pos);
__isl_give isl_set *isl_set_set_tuple_id(__isl_take isl_set *set,
	__isl_take isl_id *id);
__isl_give isl_set *isl_set_reset_tuple_id(__isl_take isl_set *set);
isl_bool isl_set_has_tuple_id(__isl_keep isl_set *set);
__isl_give isl_id *isl_set_get_tuple_id(__isl_keep isl_set *set);
__isl_give isl_set *isl_set_reset_user(__isl_take isl_set *set);

int isl_set_find_dim_by_id(__isl_keep isl_set *se