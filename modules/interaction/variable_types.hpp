#ifndef PROTOMESH_VARIABLE_TYPES_HPP
#define PROTOMESH_VARIABLE_TYPES_HPP

namespace ProtoMesh::interaction {
    /// Temperature
    /// Value in Kelvin
    /// e.g. temperature_t t = 273.5K; -> 0C
    typedef float temperature_t;

    /// Brightness
    /// Value equals percentage
    /// e.g. brightness_t b = 0.5; -> 50% brightness
    typedef float brightness_t;
}

#endif //PROTOMESH_VARIABLE_TYPES_HPP
