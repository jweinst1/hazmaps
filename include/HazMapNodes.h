#ifndef HAZ_MAP_NODES_HEADER
#define HAZ_MAP_NODES_HEADER

#include <atomic>
#include <thread>
#include <chrono>
#include <cstdint>
#include <limits>
#include <cassert>
#include <cstdio>


namespace HazMap {
	struct HazardNode {
		std::atomic<void*> ptr = nullptr;
		std::atomic<unsigned> refcnt = 1;
		// these are never deleted.
		struct HazardNode* next;

		// Only safe to use from an existing reference.
		void incRef() {
			refcnt.fetch_add(1);
		}

		// meant for lookups
		bool incRefChecked() {
			unsigned seen = refcnt.load();
			if (!seen)
				return false;
			unsigned desired = seen + 1;
			while(!refcnt.compare_exchange_weak(seen, desired)) {
				if (!seen)
					return false;
				desired = seen + 1;
			}
			return true;
		}

		void* decRef() {
			unsigned seen = refcnt.load();
			if (!seen) {
				return nullptr;
			}
			unsigned desired = seen - 1;
			while(!refcnt.compare_exchange_weak(seen, desired)) {
				if (!seen)
					return nullptr;
				desired = seen - 1;
			}
			if (seen == 1) {
				void* to_delete = ptr.exchange(nullptr);
				return to_delete;
			}
			return nullptr;
		}
	};
}


#endif