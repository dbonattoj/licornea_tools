#include "border.h"

namespace tlz {

json encode_border(const border& bord) {
	json j_bord = json::object();
	j_bord["top"] = bord.top;
	j_bord["left"] = bord.left;
	j_bord["right"] = bord.right;
	j_bord["bottom"] = bord.bottom;
	return j_bord;
}


border decode_border(const json& j_bord) {
	border bord;
	bord.top = get_or(j_bord, "top", 0);
	bord.left = get_or(j_bord, "left", 0);
	bord.right = get_or(j_bord, "right", 0);
	bord.bottom = get_or(j_bord, "bottom", 0);
	return bord;
}


}
