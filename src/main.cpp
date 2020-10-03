#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "pybind11_glm.hpp"

#include <chrono>
#include <functional>
#include <iostream>
#include <string>

#include "base.h"
#include "heightmap.h"
#include "triangulator.h"

namespace py = pybind11;

struct PydelatinTriangulator {
    PydelatinTriangulator(
      const std::vector<float> &data, const int width, const int height,
      const float maxError, const float zScale, const float zExaggeration,
      const int maxTriangles, const int maxPoints, const bool level,
      const bool invert, const int blurSigma, const float gamma,
      const int borderSize, const float borderHeight, const float baseHeight
    ) :
      data(data), width(width), height(height), maxError(maxError), zScale(zScale),
      zExaggeration(zExaggeration), maxTriangles(maxTriangles), maxPoints(maxPoints),
      level(level), invert(invert), blurSigma(blurSigma), gamma(gamma),
      borderSize(borderSize), borderHeight(borderHeight), baseHeight(baseHeight)
      { }

    void setWidth(const int &width_) { width = width_; }
    const int &getWidth() const { return width; }

    void setHeight(const int &height_) { height = height_; }
    const int &getHeight() const { return height; }

    void setMaxError(const float &maxError_) { maxError = maxError_; }
    const float &getMaxError() const { return maxError; }

    void setData(const std::vector<float> &data_) { data = data_; }

    // NOTE: I _want_ to be able to return a py:array_t<glm:ivec3>
    // That doesn't seem to work though
    const std::vector<glm::vec3> &getPoints() const { return points; }
    const std::vector<glm::ivec3> &getTriangles() const { return triangles; }

    void run() {
        const auto hm = std::make_shared<Heightmap>(width, height, data);
        int w = hm->Width();
        int h = hm->Height();

        // auto level heightmap
        if (level) {
            hm->AutoLevel();
        }

        // invert heightmap
        if (invert) {
            hm->Invert();
        }

        // blur heightmap
        if (blurSigma > 0) {
            hm->GaussianBlur(blurSigma);
        }

        // apply gamma curve
        if (gamma > 0) {
            hm->GammaCurve(gamma);
        }

        // add border
        if (borderSize > 0) {
            hm->AddBorder(borderSize, borderHeight);
        }

        Triangulator tri(hm);
        tri.Run(maxError, maxTriangles, maxPoints);
        points = tri.Points(zScale * zExaggeration);
        triangles = tri.Triangles();

        // add base
        if (baseHeight > 0) {
            const float z = -baseHeight * zScale * zExaggeration;
            AddBase(points, triangles, w, h, z);
        }
    }

    int width;
    int height;
    float maxError;
    float zScale;
    float zExaggeration;
    int maxTriangles;
    int maxPoints;
    bool level;
    bool invert;
    int blurSigma;
    float gamma;
    int borderSize;
    float borderHeight;
    float baseHeight;

    std::vector<float> data;
    std::vector<glm::vec3> points;
    std::vector<glm::ivec3> triangles;
};

PYBIND11_MODULE(_pydelatin, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    py::class_<PydelatinTriangulator>(m, "PydelatinTriangulator")
        .def(py::init<
          const std::vector<float> &, const int, const int,
          const float, const float, const float,
          const int, const int, const bool, const bool, const int,
          const float, const int, const float, const float
          >())
        .def("setWidth", &PydelatinTriangulator::setWidth)
        .def("getWidth", &PydelatinTriangulator::getWidth)
        .def("setHeight", &PydelatinTriangulator::setHeight)
        .def("getHeight", &PydelatinTriangulator::getHeight)
        .def("setMaxError", &PydelatinTriangulator::setMaxError)
        .def("getMaxError", &PydelatinTriangulator::getMaxError)
        .def("setData", &PydelatinTriangulator::setData)
        .def("getPoints", &PydelatinTriangulator::getPoints)
        .def("getTriangles", &PydelatinTriangulator::getTriangles)
        .def("run", &PydelatinTriangulator::run)
        ;

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
