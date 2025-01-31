#ifndef _IO_H_
#define _IO_H_

#include "mesh.h"
#include "prompt.h"
#include "shock.h"
#include "synchrotron.h"
void printArray(Array const& arr);
void output(SynPhotonsMesh const& syn_ph, std::string const& filename);
void output(SynElectronsMesh const& syn_e, std::string const& filename);
void output(PromptPhotonsMesh const& prompt_pj, std::string const& filename);
void output(Shock const& shock, std::string const& filename);
void output(Coord const& coord, std::string const& filename);
void output(MeshGrid3d const& array, std::string const& filename);
void output(MeshGrid const& grid, std::string const& filename);
void output(Array const& array, std::string const& filename);
void output(MeshGrid3d const& array, std::string const& filename, double unit);
void output(MeshGrid const& grid, std::string const& filename, double unit);
void output(Array const& array, std::string const& filename, double unit);
#endif