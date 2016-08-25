//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

// マトリクスのコピー
inline void CopyMatrix(Matrix44* dst, const MMatrix& src)
{
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			dst->m[i][j] = static_cast<float>(src[i][j]);
		}
	}
}
inline void CopyMatrix(Matrix44* dst, const MFloatMatrix& src)
{
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			dst->m[i][j] = static_cast<float>(src[i][j]);
		}
	}
}

// plugからベクトルを取得
inline void GetVectorByPlug(float* out, const MPlug& plug)
{
	out[0] = plug.child(0).asFloat();
	out[1] = plug.child(1).asFloat();
	out[2] = plug.child(2).asFloat();
}


// DagPathからワールドマトリクスを取得
inline void GetWorldMatrixByDagPath(const MDagPath& path, Matrix44* out, bool transpose = false)
{
	MStatus s;
	MMatrix matrix = path.inclusiveMatrix(&s);
	if (!s) return;

	if (transpose) {
		for (int32_t i = 0; i < 4; i++) {
			for (int32_t j = 0; j < 4; j++) {
				out->m[i][j] = static_cast<float>(matrix[j][i]);
			}
		}
	} else {
		for (int32_t i = 0; i < 4; i++) {
			for (int32_t j = 0; j < 4; j++) {
				out->m[i][j] = static_cast<float>(matrix[i][j]);
			}
		}
	}
}
