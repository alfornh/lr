#include "id_set.h"

#include "utils.h"

ID IDSet::get() {
  ID id;
  IDContainer::iterator beg;

  LOCK_GUARD_MUTEX_BEGIN(_mutex_ids)
  beg = _ids.begin();
  if ( beg != _ids.end() ) {
    id = *beg;
    _ids.erase(beg);
    return id;
  }

  if (_current_id >= _max) {
    return INVALIDID;
  }

  id = _current_id + INCRE_STEP;
  if (id >= _max || id < _current_id) {
    id = _max;
  }

  for (ID i = _current_id + 1; i <= id; ++i) {
    _ids.insert(i);
  }
  _current_id = id;

  beg = _ids.begin();
  if (beg == _ids.end()) {
  //if (_ids.empty()) {
    return INVALIDID;
  }

  id = *beg;
  _ids.erase(beg);

  return id;

  LOCK_GUARD_MUTEX_END
}

void IDSet::put(ID id) {
  LOCK_GUARD_MUTEX_BEGIN(_mutex_ids)
  _ids.insert(id);
  LOCK_GUARD_MUTEX_END
}
