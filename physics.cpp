//struct Physics {
//	int pos_x;
//	int pos_y;
//};
//
//inline float 
//collision_x(float x, int half_size_x, int terrain[2][2], int terrain_width) {
//	//XDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
//	for (int i = 0; i < *(&terrain + 1) - terrain; i++) {
//		if (x - half_size_x < terrain[i][1] + terrain_width) return terrain[i][1];
//		if (x + half_size_x > terrain[i][1] - terrain_width) return terrain[i][1];
//	}
//	return x;
//}
//
//inline float
//collision_y(float y, int half_size_y, int terrain[2][2], int terrain_width) {
//	//XDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
//	for (int i = 0; i < *(&terrain + 1) - terrain; i++) {
//		if (y - half_size_y < terrain[i][1]) return terrain[i][1];
//		if (y + half_size_y > terrain[i][1]) return terrain[i][1];
//	}
//	return y;
//}