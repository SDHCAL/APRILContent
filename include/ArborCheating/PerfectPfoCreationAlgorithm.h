  /// \file PerfectPfoCreationAlgorithm.h
/*
 *
 * PerfectPfoCreationAlgorithm.h header template automatically generated by a class generator
 * Creation date : ven. mars 20 2015
 *
 * This file is part of ArborContent libraries.
 *
 * ArborContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 *
 * ArborContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author John Marshall
 */


#ifndef ARBOR_PERFECT_PFO_CREATION_ALGORITHM_H
#define ARBOR_PERFECT_PFO_CREATION_ALGORITHM_H

#include "Pandora/Algorithm.h"

#include "Api/PandoraContentApi.h"

namespace arbor_content
{

/**
 *  @brief  PerfectPfoCreationAlgorithm class
 */
class PerfectPfoCreationAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

    /**
     *  @brief Default constructor
     */
    PerfectPfoCreationAlgorithm();

private:
    typedef PandoraContentApi::ParticleFlowObject::Parameters PfoParameters;

    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief  Create particle flow objects starting from tracks in the main tracker. The pfos will account for associated
     *          daughter/sibling tracks and associated calorimeter clusters.
     */
    pandora::StatusCode CreateTrackBasedPfos() const;

    /**
     *  @brief  Add relevant tracks and clusters to a track-based pfo
     * 
     *  @param  pTrack address of the track to consider
     *  @param  pfoParameters the pfo parameters to populate
     *  @param  readSiblingInfo whether to read sibling track information (set to false to avoid multiple counting)
     */
    pandora::StatusCode PopulateTrackBasedPfo(const pandora::Track *const pTrack, PfoParameters &pfoParameters, const bool readSiblingInfo = true) const;

    /**
     *  @brief  Set the basic parameters for a track-based pfo
     * 
     *  @param  pTrack address of the track to consider
     *  @param  pfoParameters the pfo parameters to populate
     */
    pandora::StatusCode SetTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Set the parameters for a track-based pfo, where pfo target consists of two or more sibling tracks
     * 
     *  @param  pTrack address of the track to consider
     *  @param  pfoParameters the pfo parameters to populate
     */
    pandora::StatusCode SetSiblingTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Set the parameters for a track-based pfo, where the pfo target has one or more daughter tracks
     * 
     *  @param  pTrack address of the track to consider
     *  @param  pfoParameters the pfo parameters to populate
     */
    pandora::StatusCode SetDaughterTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Set the parameters for a simple track-based pfo, where the track has no associations with other tracks
     * 
     *  @param  pTrack address of the track to consider
     *  @param  pfoParameters the pfo parameters to populate
     */
    pandora::StatusCode SetSimpleTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const;

    /**
     *  @brief  Create particle flow objects corresponding to neutral particles, These pfos consist only of clusters that have no
     *          associated tracks.
     */
    pandora::StatusCode CreateNeutralPfos() const;

    /**
     *  @brief  Get the energy-weighted centroid for a specified cluster, calculated over a particular pseudo layer range
     * 
     *  @param  pCluster address of the cluster to consider
     *  @param  pseudoLayer the inner pseudo layer of interest
     *  @param  pseudoLayer the outer pseudo layer of interest
     * 
     *  @return The energy-weighted centroid, returned by value
     */
    const pandora::CartesianVector GetEnergyWeightedCentroid(const pandora::Cluster *const pCluster, const unsigned int innerPseudoLayer,
        const unsigned int outerPseudoLayer) const;

    pandora::StatusCode TrackCollection(const pandora::MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const;

	void SetPfoParametersFromTracks(int &nTracksUsed, PfoParameters &pfoParameters) const;
    void SetPfoParametersFromClusters(const pandora::MCParticle *const pPfoTarget, const int nTracksUsed, PfoParameters &pfoParameters) const;
    pandora::StatusCode SetPfoParametersFromClusters() const;
    pandora::StatusCode TrackClusterAssociation(const pandora::MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const;

    std::string     m_outputPfoListName;                    ///< The output pfo list name
    std::string     m_associationAlgorithmName;             ///< The name of track-cluster alg
	bool            m_collapsedMCParticlesToPfoTarget;

    //pandora::StringVector   m_associationAlgorithms;        ///< The ordered list of track-cluster associations algorithm to run
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *PerfectPfoCreationAlgorithm::Factory::CreateAlgorithm() const
{
    return new PerfectPfoCreationAlgorithm();
}

}

#endif 
