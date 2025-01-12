// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// Code adapted from Bullet3/examples/Importers/ImportURDFDemo ...

#pragma once

#include <btBulletDynamicsCommon.h>
#include "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h"
#include "esp/physics/URDFImporter.h"
namespace esp {

namespace physics {

/**
 * @brief Structure to hold joint limit constraint info during construction.
 */
struct JointLimitConstraintInfo {
  JointLimitConstraintInfo(int _dof,
                           float _lowerLimit,
                           float _upperLimit,
                           btMultiBodyConstraint* _con) {
    dof = _dof;
    lowerLimit = _lowerLimit;
    upperLimit = _upperLimit;
    con = _con;
  }
  int dof;
  float lowerLimit, upperLimit;
  btMultiBodyConstraint* con;
};

/**
 * @brief Structure to hold construction time multi-body data.
 */
struct URDF2BulletCached {
  URDF2BulletCached()
      : m_currentMultiBodyLinkIndex(-1),
        m_bulletMultiBody(0),
        m_totalNumJoints1(0) {}
  // these arrays will be initialized in the 'InitURDF2BulletCache'

  std::vector<int> m_urdfLinkParentIndices;
  std::vector<int> m_urdfLinkIndices2BulletLinkIndices;
  std::vector<btTransform> m_urdfLinkLocalInertialFrames;

  int m_currentMultiBodyLinkIndex;

  class btMultiBody* m_bulletMultiBody;

  std::map<int, JointLimitConstraintInfo> m_jointLimitConstraints;

  // this will be initialized in the constructor
  int m_totalNumJoints1;
  int getParentUrdfIndex(int linkIndex) const {
    return m_urdfLinkParentIndices[linkIndex];
  }
  int getMbIndexFromUrdfIndex(int urdfIndex) const {
    Corrade::Utility::Debug() << "  ::getMbIndexFromUrdfIndex";
    if (urdfIndex == -2)
      return -2;
    return m_urdfLinkIndices2BulletLinkIndices[urdfIndex];
  }

  void registerMultiBody(int urdfLinkIndex,
                         class btMultiBody* body,
                         const btTransform& worldTransform,
                         btScalar mass,
                         const btVector3& localInertiaDiagonal,
                         const class btCollisionShape* compound,
                         const btTransform& localInertialFrame) {
    m_urdfLinkLocalInertialFrames[urdfLinkIndex] = localInertialFrame;
  }
};

/**
 * @brief Derived importer class for loading URDF into Bullet physics
 */
class BulletURDFImporter : public URDFImporter {
 public:
  BulletURDFImporter(esp::assets::ResourceManager& resourceManager)
      : URDFImporter(resourceManager){};

  ~BulletURDFImporter() override = default;

  btCollisionShape* convertURDFToCollisionShape(
      const struct io::URDF::CollisionShape* collision);

  btCompoundShape* convertLinkCollisionShapes(
      int linkIndex,
      const btTransform& localInertiaFrame);

  int getCollisionGroupAndMask(int linkIndex,
                               int& colGroup,
                               int& colMask) const;

  /////////////////////////////////////
  // multi-body construction functions
  void InitURDF2BulletCache(URDF2BulletCached& cache, int flags);

  void ComputeTotalNumberOfJoints(URDF2BulletCached& cache, int linkIndex);

  void ComputeParentIndices(URDF2BulletCached& cache,
                            int urdfLinkIndex,
                            int urdfParentIndex);

  Magnum::Matrix4 ConvertURDF2BulletInternal(
      URDF2BulletCached& cache,
      int urdfLinkIndex,
      const Magnum::Matrix4& parentTransformInWorldSpace,
      btMultiBodyDynamicsWorld* world1,
      int flags,
      std::map<int, std::unique_ptr<btCollisionShape>>& linkCollisionShapes);
};

void processContactParameters(const io::URDF::LinkContactInfo& contactInfo,
                              btCollisionObject* col);

}  // namespace physics
}  // namespace esp
