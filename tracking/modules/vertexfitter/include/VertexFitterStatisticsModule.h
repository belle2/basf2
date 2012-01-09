#ifndef VERTEXFITTERSTATISTICSMODULE_H
#define VERTEXFITTERSTATISTICSMODULE_H

#include <framework/core/Module.h>

#include <TH1D.h>

namespace Belle2 {

  class VertexFitterStatisticsModule : public Module {

  public:
    VertexFitterStatisticsModule();
    virtual ~VertexFitterStatisticsModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    std::string m_RAVEVerticesColName;         ///< rave::Vertex instance collection name.
    std::string m_MCParticlesColName;          ///< MCParticle instances collection name.
    std::string m_GFTrackCandsColName;         ///< GFTrackCand instances collection name.
    std::string m_GFTracksColName;             ///< GFTrack instance collection name.
    std::string m_GFTrackToGFTrackCandColName; ///< Collection of relations of GFTrack instances to GFTrackCand instances.
    std::string m_resultFilePath;              ///< The path and file name of the files that are going to be saved, without any file extension.
    std::string m_steeringFile;                ///< The path to the steering file used.

    TH1D* trackMomentumHistogram;              ///< The histogram describing the magnitude of the track momentum vector.
    TH1D* trackMomentumXHistogram;             ///< The histogram describing the X component of the track momentum vector.
    TH1D* trackMomentumYHistogram;             ///< The histogram describing the Y component of the track momentum vector.
    TH1D* trackMomentumZHistogram;             ///< The histogram describing the Z component of the track momentum vector.
    TH1D* vertexDisplacementHistogram;         ///< The histogram describing the spatial displacement of the reconstructed vertex from its real position.
    TH1D* vertexDisplacementXHistogram;        ///< The histogram describing the difference along the x Axis from the true vertex to the calculated vertex.
    TH1D* vertexDisplacementYHistogram;        ///< The histogram describing the difference along the y Axis from the true vertex to the calculated vertex.
    TH1D* vertexDisplacementZHistogram;        ///< The histogram describing the difference along the z Axis from the true vertex to the calculated vertex.
    TH1D* standardisedResidualXHistogram;      ///< The histogram describing the standardised residual in the X coordinate of the vertex.
    TH1D* standardisedResidualYHistogram;      ///< The histogram describing the standardised residual in the Y coordinate of the vertex.
    TH1D* standardisedResidualZHistogram;      ///< The histogram describing the standardised residual in the Z coordinate of the vertex.
    TH1D* trueVertexXHistogram;                ///< The histogram describing the X coordinate of the true vertex.
    TH1D* trueVertexYHistogram;                ///< The histogram describing the Y coordinate of the true vertex.
    TH1D* trueVertexZHistogram;                ///< The histogram describing the Z coordinate of the true vertex.
    TH1D* chiSquaredHistogram;                 ///< The histogram describing chi^2 of the calculated vertices.
    TH1D* probHistogram;                       ///< The histogram describing the TMath::prob(chi2, ndf) values of the calculated vertices.
    TH1D* trackProbHistogram;                  ///< The histogram describing the TMath::prob(chi2, ndf) values of the tracks.
  };
}

#endif
