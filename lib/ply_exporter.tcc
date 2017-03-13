#include "ply_exporter.h"

namespace tlz {

template<typename Iterator>
void ply_exporter::write(Iterator begin, Iterator end) {
	if(full_) {
		if(ascii_) for(Iterator it = begin; it != end; ++it) write_full_ascii_(*it);
		else for(Iterator it = begin; it != end; ++it) write_full_binary_(*it);
	} else {
		if(ascii_) for(Iterator it = begin; it != end; ++it) write_ascii_(*it);
		else for(Iterator it = begin; it != end; ++it) write_binary_(*it);
	}
}

}
