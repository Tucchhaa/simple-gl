#include "converter.h"

#include <glm/detail/type_quat.hpp>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>

namespace SimpleGL {

btVector3 Converter::toBt(const glm::vec3& v) { return btVector3(v.x, v.y, v.z); }
btQuaternion Converter::toBt(const glm::quat& q) { return btQuaternion(q.x, q.y, q.z, q.w); }

glm::vec3 Converter::toGlm(const btVector3& v) { return glm::vec3(v.x(), v.y(), v.z()); }
glm::quat Converter::toGlm(const btQuaternion& q) { return glm::quat(q.w(), q.x(), q.y(), q.z()); }

}
