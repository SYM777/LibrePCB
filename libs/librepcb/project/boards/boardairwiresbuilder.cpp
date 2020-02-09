/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "boardairwiresbuilder.h"

#include "../circuit/circuit.h"
#include "../circuit/componentsignalinstance.h"
#include "../circuit/netsignal.h"
#include "../project.h"
#include "board.h"
#include "items/bi_footprintpad.h"
#include "items/bi_netline.h"
#include "items/bi_netpoint.h"
#include "items/bi_netsegment.h"
#include "items/bi_plane.h"
#include "items/bi_via.h"

#include <librepcb/common/algorithm/airwiresbuilder.h>
#include <librepcb/common/graphics/graphicslayer.h>
#include <librepcb/library/pkg/footprintpad.h>

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace project {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

BoardAirWiresBuilder::BoardAirWiresBuilder(const Board&     board,
                                           const NetSignal& netsignal) noexcept
  : mBoard(board), mNetSignal(netsignal) {
}

BoardAirWiresBuilder::~BoardAirWiresBuilder() noexcept {
}

/*******************************************************************************
 *  General Methods
 ******************************************************************************/

QVector<QPair<Point, Point>> BoardAirWiresBuilder::buildAirWires() const {
  AirWiresBuilder                     builder;
  QHash<const BI_NetLineAnchor*, int> anchorMap;
  QHash<int, QString>                 layerMap;

  // pads
  foreach (ComponentSignalInstance* cmpSig, mNetSignal.getComponentSignals()) {
    Q_ASSERT(cmpSig);
    foreach (BI_FootprintPad* pad, cmpSig->getRegisteredFootprintPads()) {
      if (&pad->getBoard() != &mBoard) continue;
      int id         = builder.addPoint(pad->getPosition());
      anchorMap[pad] = id;
      if (pad->getLibPad().getBoardSide() ==
          library::FootprintPad::BoardSide::THT) {
        layerMap[id] = QString();  // on all layers
      } else {
        layerMap[id] = pad->getLayerName();
      }
    }
  }

  // vias, netpoints, netlines
  foreach (const BI_NetSegment* netsegment, mNetSignal.getBoardNetSegments()) {
    Q_ASSERT(netsegment);
    if (&netsegment->getBoard() != &mBoard) continue;
    foreach (const BI_Via* via, netsegment->getVias()) {
      Q_ASSERT(via);
      int id         = builder.addPoint(via->getPosition());
      anchorMap[via] = id;
      layerMap[id]   = QString();  // on all layers
    }
    foreach (const BI_NetPoint* netpoint, netsegment->getNetPoints()) {
      Q_ASSERT(netpoint);
      if (const GraphicsLayer* layer = netpoint->getLayerOfLines()) {
        int id              = builder.addPoint(netpoint->getPosition());
        anchorMap[netpoint] = id;
        layerMap[id]        = layer->getName();
      }
    }
    foreach (const BI_NetLine* netline, netsegment->getNetLines()) {
      Q_ASSERT(netline);
      Q_ASSERT(anchorMap.contains(&netline->getStartPoint()));
      Q_ASSERT(anchorMap.contains(&netline->getEndPoint()));
      builder.addEdge(anchorMap[&netline->getStartPoint()],
                      anchorMap[&netline->getEndPoint()]);
    }
  }

  // determine connections made by planes
  foreach (const BI_Plane* plane, mNetSignal.getBoardPlanes()) {
    Q_ASSERT(plane);
    if (&plane->getBoard() != &mBoard) continue;
    foreach (const Path& fragment, plane->getFragments()) {
      int lastId = -1;
      for (const auto& point : builder.getPoints()) {
        QString pointLayer = layerMap[point.id];
        if (pointLayer.isNull() || (pointLayer == plane->getLayerName())) {
          Point p(point.x, point.y);
          if (fragment.toQPainterPathPx().contains(p.toPxQPointF())) {
            if (lastId >= 0) {
              builder.addEdge(lastId, point.id);
            }
            lastId = point.id;
          }
        }
      }
    }
  }

  return builder.buildAirWires();
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace project
}  // namespace librepcb
