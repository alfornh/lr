#ifndef _ID_SET_H__
#define _ID_SET_H__

#include <memory>
#include <mutex>
#include <set>

#include "plt/type-inc.h"

#define ID_MIN     1

#define INCRE_STEP 1000
#define INVALIDID  0

class IDSet {
public:
  typedef std::shared_ptr<IDSet> ptr;

  IDSet() {
    _min = ID_MIN;
    _max = ID_MAX;
    _current_id = INVALIDID;
  }

  //int init();

  ID get();
  void put(ID id);

  void set_max(ID id) {
    _max = id;
  }

private:
  typedef std::set<ID> IDContainer;
  IDContainer _ids;
  std::mutex _mutex_ids;

  ID _min;
  ID _current_id;
  ID _max;
};

static std::shared_ptr<IDSet> _socket_idset = std::make_shared<IDSet>();
#define PSIDSET _socket_idset

#endif//_ID_SET_H__
