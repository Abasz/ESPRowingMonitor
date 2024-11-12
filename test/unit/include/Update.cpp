#include "./fakeit.hpp"

#include "./Update.h"

fakeit::Mock<UpdateClass> mockUpdate;
UpdateClass &Update = mockUpdate.get();