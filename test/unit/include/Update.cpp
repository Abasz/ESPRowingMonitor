// NOLINTBEGIN(bugprone-throwing-static-initialization)
#include "fakeit.hpp"

#include "./Update.h"

fakeit::Mock<UpdateClass> mockUpdate;
UpdateClass &Update = mockUpdate.get();
// NOLINTEND(bugprone-throwing-static-initialization)