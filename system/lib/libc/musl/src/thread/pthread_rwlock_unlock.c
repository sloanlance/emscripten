#include "pthread_impl.h"

int pthread_rwlock_unlock(pthread_rwlock_t *rw)
{
	int val, cnt, waiters, new;

#ifdef __EMSCRIPTEN__
	/// XXX Emscripten: The spec allows detecting when multiple write locks would deadlock, which we do here to avoid hangs.
	/// Mark this thread to not own the write lock anymore.
	if (rw->_rw_wr_owner == (int)pthread_self()) rw->_rw_wr_owner = 0;
#endif

	do {
		val = rw->_rw_lock;
		cnt = val & 0x7fffffff;
		waiters = rw->_rw_waiters;
		new = (cnt == 0x7fffffff || cnt == 1) ? 0 : val-1;
	} while (a_cas(&rw->_rw_lock, val, new) != val);

	if (!new && (waiters || val<0))
		__wake(&rw->_rw_lock, cnt, 0);

	return 0;
}
