#pragma once

#include <glm/glm.hpp>
#include <LinearMath/btVector3.h>

class btVector3;
class btQuaternion;

namespace SimpleGL {

class Converter {
public:
    static btVector3 toBt(const glm::vec3& v);
    static btQuaternion toBt(const glm::quat& q);

    static glm::vec3 toGlm(const btVector3& v);
    static glm::quat toGlm(const btQuaternion& q);
};

}

