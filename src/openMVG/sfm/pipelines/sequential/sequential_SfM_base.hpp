// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2015 Pierre MOULON, Ricardo FABBRI and Gabriel ANDRADE.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_SFM_LOCALIZATION_SEQUENTIAL_SFM_BASE_HPP
#define OPENMVG_SFM_LOCALIZATION_SEQUENTIAL_SFM_BASE_HPP

#include <set>
#include <string>
#include <vector>

#include "openMVG/multiview/multiview_match_constraint.hpp"
#include "openMVG/multiview/triangulation_method.hpp"
#include "openMVG/multiview/solver_resection.hpp"
#include "openMVG/sfm/base/sfm_engine.hpp"
#include "openMVG/cameras/cameras.hpp"
#include "openMVG/tracks/tracks.hpp"
#include "third_party/histogram/histogram_raw.hpp"

namespace htmlDocument { class htmlDocumentStream; }
namespace histogramming {template <typename T> class Histogram; }

namespace openMVG {
namespace sfm {

struct Features_Provider;
struct Matches_Provider;

/// Sequential SfM Pipeline Reconstruction Engine.
/// This Base class has functions that are mostly done,
/// and need no recompiling
/// Its only purpose for now is to speed up recompiling cycle
/// because there are too many lines of code in the engine
class SequentialSfMReconstructionEngineBase : public ReconstructionEngine
{
public:

  SequentialSfMReconstructionEngineBase(
    const SfM_Data & sfm_data,
    const std::string & soutDirectory,
    const std::string & loggingFile = "");

  ~SequentialSfMReconstructionEngineBase() override;

  void SetFeaturesProvider(Features_Provider * provider);
  void SetMatchesProvider(Matches_Provider * provider);

  void SetMultiviewMatchConstraint(MultiviewMatchConstraint c) 
  { multiview_match_constraint_ = c; }

  bool UseOrientedConstraint() const { return multiview_match_constraint_ == MultiviewMatchConstraint::ORIENTATION; }

  void setInitialPair(const Pair & initialPair) 
  { initial_pair_ = initialPair; }
  void setInitialTriplet(const Triplet & initialTriplet) 
  { initial_triplet_ = initialTriplet; }
  bool hasInitialPair() { return initial_pair_ != Pair(0,0); }
  bool hasInitialTriplet() { return initial_triplet_ != Triplet(0,0,0); }

  /// Initialize tracks
  bool InitLandmarkTracks();
  
  /// TODO(trifocal future) Automatic initial triplet selection (based on a 'baseline' computation score)
  // bool AutomaticInitialTripletChoice(Triple & initialTriplet) const;

  /**
   * Set the default lens distortion type to use if it is declared unknown
   * in the intrinsics camera parameters by the previous steps.
   *
   * It can be declared unknown if the type cannot be deduced from the metadata.
   */
  void SetUnknownCameraType(const cameras::EINTRINSIC camType) 
  { cam_type_ = camType; }

  /// Configure the 2view triangulation method used by the SfM engine
  void SetTriangulationMethod(const ETriangulationMethod method) 
  { triangulation_method_ = method; }

  /// Configure the resetcion method method used by the Localization engine
  void SetResectionMethod(const resection::SolverType method) 
  { resection_method_ = method; }

  void SetMaximumTrifocalRansacIterations(unsigned n) 
  { maximum_trifocal_ransac_iterations_ = n; }

  unsigned MaximumTrifocalRansacIterations() 
  { return maximum_trifocal_ransac_iterations_; }

  static constexpr unsigned maximum_trifocal_ransac_iterations_DEFAULT = 100;

  void FinalStatistics();

protected:

  /// Compute the initial 3D seed (First camera: {R=Id|t=0}, second estimated {R|t} by 5 point algorithm)
  bool MakeInitialPair3D(const Pair & initialPair);

  /// Automatic initial pair selection (based on a 'baseline' computation score)
  bool AutomaticInitialPairChoice(Pair & initialPair) const;

  /// Return MSE (Mean Square Error) and a histogram of residual values.
  double ComputeResidualsHistogram(histogramming::Histogram<double> * histo) const;

  /// List the images that the greatest number of matches to the current 3D reconstruction.
  bool FindImagesWithPossibleResection(std::vector<uint32_t> & vec_possible_indexes);

  /// Bundle adjustment to refine Structure; Motion and Intrinsics
  bool BundleAdjustment();

  bool using_initial_triple() { return std::get<2>(initial_triplet_) != 0; }

  //----
  //-- Data
  //----

  // HTML logger
  std::shared_ptr<htmlDocument::htmlDocumentStream> html_doc_stream_;
  std::string sLogging_file_;
  
  // Parameter

  MultiviewMatchConstraint multiview_match_constraint_;
  Triplet initial_triplet_;
  Pair initial_pair_;
  
  cameras::EINTRINSIC cam_type_; // The camera type for the unknown cameras

  //-- Data provider
  Features_Provider  *features_provider_;
  Matches_Provider  *matches_provider_;

  // Temporary data
  openMVG::tracks::STLMAPTracks map_tracks_; // putative landmark tracks (visibility per 3D point)

  // Helper to compute if some image have some track in common
  std::unique_ptr<openMVG::tracks::SharedTrackVisibilityHelper> shared_track_visibility_helper_;

  Hash_Map<IndexT, double> map_ACThreshold_; // Per camera confidence (A contrario estimated threshold error)

  std::set<uint32_t> set_remaining_view_id_;     // Remaining camera index that can be used for resection

  ETriangulationMethod triangulation_method_ = ETriangulationMethod::DEFAULT;

  resection::SolverType resection_method_ = resection::SolverType::DEFAULT;

  unsigned maximum_trifocal_ransac_iterations_ = maximum_trifocal_ransac_iterations_DEFAULT;

private:

};

} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_LOCALIZATION_SEQUENTIAL_SFM_BASE_HPP
