#ifndef HAZ_MAP_HEADER
#define HAZ_MAP_HEADER

#include <atomic>
#include <thread>
#include <chrono>
#include <cstdint>
#include <limits>
#include <cassert>
#include <cstdio>


namespace HazMap {
	struct Node {
		std::atomic<void*> ptr = nullptr;
		std::atomic<unsigned> refcnt = 1;
		// these are never deleted.
		struct Node* next;

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

	struct List {
		std::atomic<Node*> hlist = nullptr;

		Node* findEmptySlot(void* ptr, size_t times) {
			for (size_t i = 0; i < times; ++i) {
				Node* iter = hlist.load();
				while (iter != nullptr) {
					void* slotcheck = iter->ptr.load();
					if (slotcheck != nullptr) {
						continue;
					}
					// placement logic.
					if (iter->ptr.compare_exchange_strong(slotcheck, ptr)) {
						// take away at later point
						unsigned exres = iter->refcnt.exchange(1);
						if (exres != 0) {
							printf("got %u in exres\n", exres);
							assert(0);
						}
						return iter;
					}
					iter = iter->next;
				}
			}
			return nullptr;
		}

		Node* add(void* ptr, bool tryToReuse = true) {
			Node* tryToUseExisting = tryToReuse ? findEmptySlot(ptr, 2) : nullptr;
			if (tryToUseExisting != nullptr) {
				return tryToUseExisting;
			}
			Node* newnode = new Node();
			newnode->ptr.store(ptr);
			Node* got = hlist.load();
			newnode->next = got;
			while(!hlist.compare_exchange_weak(got, newnode)) {
				newnode->next = got;
			}
			return newnode;
		}

		unsigned getRefCount(void* ptr) {
			Node* iter = hlist.load();
			while (iter != nullptr) {
				if (iter->ptr.load() == ptr)
					return iter->refcnt.load();
				iter = iter->next;
			}
			return 0;
		}

		Node* createRef(void* ptr) {
			Node* iter = hlist.load();
			while (iter != nullptr) {
				void* slotcheck = iter->ptr.load();
				if (slotcheck == ptr) {
					if (iter->incRefChecked()) {
						return iter;
					} else {
						// existed but was deleted before we could increment
						return nullptr;
					}
				}
				iter = iter->next;
			}
			return nullptr;
		}
	};
}


#endif