/***************************************************************************
     testqgsattributetable.cpp
     -------------------------
    Date                 : 2016-02-14
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsattributetabledialog.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"

/** \ingroup UnitTests
 * This is a unit test for the attribute table dialog
 */
class TestQgsAttributeTable : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeTable();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testFieldCalculation();
    void testFieldCalculationArea();

  private:
    QgisApp * mQgisApp;
};

TestQgsAttributeTable::TestQgsAttributeTable()
    : mQgisApp( nullptr )
{

}

//runs before all tests
void TestQgsAttributeTable::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsAttributeTable::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributeTable::testFieldCalculation()
{
  //test field calculation

  //create a temporary layer
  QScopedPointer< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolyline line3111;
  line3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 );
  QgsGeometry line3111G = QgsGeometry::fromPolyline( line3111 ) ;
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceMeters );

  // run length calculation
  QScopedPointer< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.data() ) );
  tempLayer->startEditing();
  dlg->runFieldCalculation( tempLayer.data(), QStringLiteral( "col1" ), QStringLiteral( "$length" ) );
  tempLayer->commitChanges();
  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 26932.156;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceFeet );
  QScopedPointer< QgsAttributeTableDialog > dlg2( new QgsAttributeTableDialog( tempLayer.data() ) );
  tempLayer->startEditing();
  dlg2->runFieldCalculation( tempLayer.data(), QStringLiteral( "col1" ), QStringLiteral( "$length" ) );
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 88360.0918635;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );
}

void TestQgsAttributeTable::testFieldCalculationArea()
{
  //test $area field calculation

  //create a temporary layer
  QScopedPointer< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );

  QgsPolyline polygonRing3111;
  polygonRing3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 ) << QgsPoint( 2520109, 2397715 ) << QgsPoint( 2520792, 2425494 ) << QgsPoint( 2484588, 2425722 );
  QgsPolygon polygon3111;
  polygon3111 << polygonRing3111;
  QgsGeometry polygon3111G = QgsGeometry::fromPolygon( polygon3111 );
  f1.setGeometry( polygon3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMeters );

  // run area calculation
  QScopedPointer< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.data() ) );
  tempLayer->startEditing();
  dlg->runFieldCalculation( tempLayer.data(), QStringLiteral( "col1" ), QStringLiteral( "$area" ) );
  tempLayer->commitChanges();
  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 1009089817.0;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 1.0 ) );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMiles );
  QScopedPointer< QgsAttributeTableDialog > dlg2( new QgsAttributeTableDialog( tempLayer.data() ) );
  tempLayer->startEditing();
  dlg2->runFieldCalculation( tempLayer.data(), QStringLiteral( "col1" ), QStringLiteral( "$area" ) );
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 389.6117565069;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );
}

QTEST_MAIN( TestQgsAttributeTable )
#include "testqgsattributetable.moc"
