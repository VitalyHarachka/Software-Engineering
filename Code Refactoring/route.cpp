#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "geometry.h"
#include "xmlparser.h"
#include "route.h"

using namespace GPS;

std::string Route::name() const
{
    return routeName.empty() ? "Unnamed Route" : routeName;
}

unsigned int Route::numPositions() const
{
    return (unsigned int)positions.size();
}

metres Route::totalLength() const
{
    // The total length of the Route; this is the sum of the distances between successive route points.
    return routeLength;
}

metres Route::netLength() const
{
    Position firstPosition = positions[0];
    Position lastPosition = positions[positions.size() - 1];

    if (areSameLocation(firstPosition, lastPosition))
    {
        return 0;
    }

    return Position::distanceBetween(firstPosition, lastPosition);
}

metres Route::totalHeightGain() const
{
    assert(!positions.empty());

    metres total = 0.0;
    for (unsigned int i = 1; i < numPositions(); ++i)
    {
        metres deltaV = positions[i].elevation() - positions[i - 1].elevation();
        if (deltaV > 0.0) total += deltaV; // ignore negative height differences
    }
    return total;
}

metres Route::netHeightGain() const
{
    assert(!positions.empty());

    metres deltaV = positions.back().elevation() - positions.front().elevation();
    return std::max(deltaV, 0.0); // ignore negative height differences
}

degrees Route::minLatitude() const
{
    if (positions.empty()) {
        throw std::out_of_range("Cannot get the minimum latitude of an empty route");
    }

    degrees lowestLatitude = positions[0].latitude();

    double epsilon = 0.0001;

    for (int i = 0; i < positions.size(); i++)
    {
        if ((positions[i].latitude() - lowestLatitude) < epsilon)
        {
            lowestLatitude = positions[i].latitude();
        }
    }

    return lowestLatitude;
}

degrees Route::maxLatitude() const
{
    degrees currentMax = positions[0].latitude();

    for (int i = 0; i < positions.size(); i++) {
        if (positions[i].latitude() > currentMax)
            currentMax = positions[i].latitude();
    }

    return currentMax;
}

degrees Route::minLongitude() const     //MY FUNCTION
{
    assert(!positions.empty());

    degrees minLon = positions.front().longitude();
    for (const Position& pos : positions)
    {
        minLon = std::min(minLon, pos.longitude());
    }
    return minLon;
}

degrees Route::maxLongitude() const
{
    assert(!positions.empty());

    degrees maxLon = positions.front().longitude();
    for (const Position& pos : positions)
    {
        maxLon = std::max(maxLon, pos.longitude());
    }
    return maxLon;

}

metres Route::minElevation() const
{
    assert(!positions.empty());

    degrees minEle = positions.front().elevation();
    for (const Position& pos : positions)
    {
        minEle = std::min(minEle, pos.elevation());
    }
    return minEle;
}

metres Route::maxElevation() const
{
    assert(!positions.empty());

    degrees maxEle = positions.front().elevation();
    for (const Position& pos : positions)
    {
        maxEle = std::max(maxEle, pos.elevation());
    }
    return maxEle;
}

degrees Route::maxGradient() const
{
    assert(!positions.empty());

    if (positions.size() == 1) return 0.0;

    degrees maxGrad = -halfRotation / 2; // minimum possible value
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres deltaH = Position::distanceBetween(positions[i], positions[i - 1]);
        metres deltaV = positions[i].elevation() - positions[i - 1].elevation();
        degrees grad = radToDeg(std::atan(deltaV / deltaH));
        maxGrad = std::max(maxGrad, grad);
    }
    return maxGrad;
}

degrees Route::minGradient() const
{
    assert(!positions.empty());

    if (positions.size() == 1) return 0.0;

    degrees minGrad = halfRotation / 2; // maximum possible value
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres deltaH = Position::distanceBetween(positions[i], positions[i - 1]);
        metres deltaV = positions[i].elevation() - positions[i - 1].elevation();
        degrees grad = radToDeg(std::atan(deltaV / deltaH));
        minGrad = std::min(minGrad, grad);
    }
    return minGrad;
}

degrees Route::steepestGradient() const
{
    assert(!positions.empty());

    if (positions.size() == 1) return 0.0;

    degrees maxGrad = -halfRotation / 2; // minimum possible value
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres deltaH = Position::distanceBetween(positions[i], positions[i - 1]);
        metres deltaV = positions[i].elevation() - positions[i - 1].elevation();
        degrees grad = radToDeg(std::atan(deltaV / deltaH));
        maxGrad = std::max(maxGrad, std::abs(grad));
    }
    return maxGrad;
}

Position Route::operator[](unsigned int idx) const
{
    return positions.at(idx);
}

Position Route::findPosition(const std::string & soughtName) const
{
    auto nameIt = std::find(positionNames.begin(), positionNames.end(), soughtName);

    if (nameIt == positionNames.end())
    {
        throw std::out_of_range("No position with that name found in the route.");
    }
    else
    {
        return positions[std::distance(positionNames.begin(), nameIt)];
    }
}

std::string Route::findNameOf(const Position & soughtPos) const
{
    auto posIt = std::find_if(positions.begin(), positions.end(),
        [&](const Position& pos) {return areSameLocation(pos, soughtPos); });

    if (posIt == positions.end())
    {
        throw std::out_of_range("Position not found in route.");
    }
    else
    {
        return positionNames[std::distance(positions.begin(), posIt)];
    }
}

unsigned int Route::timesVisited(const std::string & soughtName) const
{
    unsigned int timesVisited{ 0 };

    try {

        Position position = this->findPosition(soughtName);
        for (const auto &i : positions)
            if (areSameLocation(i, position)) timesVisited++;

    }
    catch (const std::out_of_range& e) {}

    return timesVisited;
}

unsigned int Route::timesVisited(const Position & soughtPos) const
{
    unsigned int timesVisited{ 0 };

    for (const auto &i : positions)
        if (areSameLocation(i, soughtPos)) timesVisited++;

    return timesVisited;
}

std::string Route::buildReport() const
{
    return report;
}

//Constructs a route
//If isFileName is false then route is constructed from the data in string source
//Otherwise the route is constructed from the data contained inside the file referenced by source
Route::Route(std::string source, bool isFileName, metres granularity)
{


    this->granularity = granularity;

    if (isFileName) {  //If source is a filename, process as a file
        std::string filePath = source;
        loadFileToSource(filePath, source);
    }

    parseSource(source);
    calcRouteLength();
}

//------------------- protected methods ---------------------

bool Route::areSameLocation(const Position & p1, const Position & p2) const
{
    return (Position::distanceBetween(p1, p2) < granularity);
}

//------------------- private helper methods ---------------------

void Route::appendToReport(const std::ostringstream & value)
{
    report += value.str();
}

void Route::loadFileToSource(const std::string &filePath, std::string & source)
{

    std::string temp;
    std::ostringstream oss;
    std::ostringstream reportStr;
    std::ifstream fs(filePath);

    if (!fs.good()) {
        throw std::invalid_argument("Error opening source file '" + filePath + "'.");
    }
    reportStr << "Source file '" << filePath << "' opened okay." << std::endl;

    while (fs.good()) {
        getline(fs, temp);
        oss << temp << std::endl;
    }
    source = oss.str();

    while (fs.good()) {
        getline(fs, temp);
        oss << temp << std::endl;
    }

    source = oss.str(); //return source data loaded from file using pass by reference
    appendToReport(reportStr);
}

void Route::parseSource(std::string &source)
{
   //Rmove using "namespace std;"

    using namespace XML::Parser;

    const int MAX = 3; //array size

    std::string lat, lon, ele;
    std::string tempStorage[MAX]; //Removed temp, temp2, name and convert it into array for temp storage
    std::ostringstream reportStr;

    //unsigned int num = 0; not needed as number of posistion can be called using positionNames.size() vector

    if (!elementExists(source, "gpx")) {
        throw std::domain_error("No 'gpx' element.");
    } else {
        tempStorage[0] = getElement(source, "gpx");
        source = getElementContent(tempStorage[0]);
    }

    if (!elementExists(source, "rte")) {
        throw std::domain_error("No 'rte' element.");
    } else {
        tempStorage[0] = getElement(source, "rte");
        source = getElementContent(tempStorage[0]);
    }


    if (elementExists(source, "name")) {
        tempStorage[0] = getAndEraseElement(source, "name");
        routeName = getElementContent(tempStorage[0]);
        reportStr << "Route name is: " << routeName << std::endl;
    }

    if (!elementExists(source, "rtept")) {
        throw std::domain_error("No 'rtept' element.");
    } else {
        tempStorage[0] = getAndEraseElement(source, "rtept");
    }

    if (!attributeExists(tempStorage[0], "lat")) {
        throw std::domain_error("No 'lat' attribute.");

    } else if (!attributeExists(tempStorage[0], "lon")) {
        throw std::domain_error("No 'lon' attribute.");

    } else {
        lat = getElementAttribute(tempStorage[0], "lat");
        lon = getElementAttribute(tempStorage[0], "lon");
        tempStorage[0] = getElementContent(tempStorage[0]);
    }


    if (elementExists(tempStorage[0], "ele")) {
        tempStorage[1] = getElement(tempStorage[0], "ele");
        ele = getElementContent(tempStorage[1]);
        Position startPos = Position(lat, lon, ele);
        positions.push_back(startPos);
        reportStr << "Position added: " << startPos.toString() << std::endl;

    }
    else {
        Position startPos = Position(lat, lon);
        positions.push_back(startPos);
        reportStr << "Position added: " << startPos.toString() <<  std::endl;

    }

    if (elementExists(tempStorage[0], "name")) {
        tempStorage[0] = getElement(tempStorage[0], "name");
        tempStorage[2] = getElementContent(tempStorage[0]);
    }

    positionNames.push_back(tempStorage[2]);
    Position prevPos = positions.back(), nextPos = positions.back();

    while (elementExists(source, "rtept")) {
        tempStorage[0] = getAndEraseElement(source, "rtept");

        if (!attributeExists(tempStorage[0], "lat")) {
            throw std::domain_error("No 'lat' attribute.");
        }
        if (!attributeExists(tempStorage[0], "lon")) {
            throw std::domain_error("No 'lon' attribute.");
        }
        lat = getElementAttribute(tempStorage[0], "lat");
        lon = getElementAttribute(tempStorage[0], "lon");
        tempStorage[0] = getElementContent(tempStorage[0]);

        if (elementExists(tempStorage[0], "ele")) {
            tempStorage[1] = getElement(tempStorage[0], "ele");
            ele = getElementContent(tempStorage[1]);
            nextPos = Position(lat, lon, ele);
        }
        else nextPos = Position(lat, lon);

        if (areSameLocation(nextPos, prevPos)) {
            reportStr << "Position ignored: " << nextPos.toString() << std::endl;
        }
        else {
            if (elementExists(tempStorage[0], "name")) {
                tempStorage[1] = getElement(tempStorage[0], "name");
                tempStorage[2] = getElementContent(tempStorage[1]);
            }
            else tempStorage[2] = ""; // Fixed bug by adding this.
            positions.push_back(nextPos);
            positionNames.push_back(tempStorage[2]);
            reportStr << "Position added: " << nextPos.toString() << std::endl;

            prevPos = nextPos;
        }
    }
    reportStr << positionNames.size() << " positions added." << std::endl;
    appendToReport(reportStr);
}

void Route::calcRouteLength(void)
{

    metres deltaH, deltaV;

    routeLength = 0;

    for (size_t i = 1; i < positionNames.size(); ++i) {
        deltaH = Position::distanceBetween(positions[i - 1], positions[i]);
        deltaV = positions[i - 1].elevation() - positions[i].elevation();
        routeLength += deltaH /*removed sqrt and pow */+ pow(deltaV, 2);
    }
}

void Route::setGranularity(metres granularity)
{
    bool implemented = false;
    assert(implemented);
}





