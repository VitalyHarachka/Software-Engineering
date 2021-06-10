#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdexcept>

#include "geometry.h"
#include "xmlparser.h"
#include "track.h"

using namespace GPS;

// Note: The implementation should exploit the relationship:
//   totalTime() == restingTime() + travellingTime()

seconds Track::totalTime() const
{
    assert(! departed.empty());
    return departed.back();
}

seconds Track::restingTime() const
{
    seconds total = 0;
    assert (arrived.size() == departed.size());
    for (unsigned int i = 0; i < arrived.size(); ++i)
    {
        total += departed[i] - arrived[i];
    }
    return total;
}

seconds Track::travellingTime() const
{
    return totalTime() - restingTime();
}

speed Track::maxSpeed() const
{
    assert( positions.size() == departed.size() && positions.size() == arrived.size() );
    if (positions.size() == 1) return 0.0;

    speed ms = 0;
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres deltaH = Position::distanceBetween(positions[i],positions[i-1]);
        metres deltaV = positions[i].elevation() - positions[i-1].elevation();
        metres distance = std::sqrt(std::pow(deltaH,2) + std::pow(deltaV,2));
        seconds time = arrived[i] - departed[i-1];
        ms = std::max(ms,distance/time);
    }
    return ms;
}

speed Track::averageSpeed(bool includeRests) const
{
    seconds time = (includeRests ? totalTime() : travellingTime());
    if (time == 0) return 0;
    else return totalLength() / time;
}

speed Track::maxRateOfAscent() const
{
    assert( positions.size() == departed.size() && positions.size() == arrived.size() );
    if (positions.size() == 1) return 0.0;

    speed ms = 0;
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres height = positions[i].elevation() - positions[i-1].elevation();
        seconds time = arrived[i] - departed[i-1];
        ms = std::max(ms,height/time);
    }
    return ms;
}

speed Track::maxRateOfDescent() const
{
    assert( positions.size() == departed.size() && positions.size() == arrived.size() );
    if (positions.size() == 1) return 0.0;

    speed ms = 0;
    for (unsigned int i = 1; i < positions.size(); ++i)
    {
        metres height = positions[i-1].elevation() - positions[i].elevation();
        seconds time = arrived[i] - departed[i-1];
        ms = std::max(ms,height/time);
    }
    return ms;
}


Track::Track(std::string source, bool isFileName, metres granularity)
{
    using namespace std;
    using namespace XML::Parser;

    string mergedTrkSegs,trkseg,lat,lon,ele,time;
    const int MAX = 3;
    string tempStorage[MAX];

    seconds startTime, currentTime, timeElapsed;
    ostringstream reportStr;

   // unsigned int num; not needed as number of posistion can be called using positionNames.size() vector
    this->granularity = granularity;

    if (isFileName) {
        std::string filePath = source;
        Track::loadFileToSource(filePath, source); //file reading function obtained from route.h made public
    }


    if (! elementExists(source,"gpx")) {
        throw domain_error("No 'gpx' element.");

    } else {
        tempStorage[0] = getElement(source, "gpx");
        source = getElementContent( tempStorage[0]);
    }

    if (! elementExists(source,"trk")) {
        throw domain_error("No 'trtestk' element.");
    } else {
       tempStorage[0] = getElement(source, "trk");
       source = getElementContent( tempStorage[0]);
    }

    if (elementExists(source, "name")) {
        tempStorage[0] = getAndEraseElement(source, "name");
        routeName = getElementContent( tempStorage[0]);
        reportStr << "Track name is: " << routeName << endl;
    }


    while (elementExists(source, "trkseg")) {
         tempStorage[0] = getAndEraseElement(source, "trkseg");
        trkseg = getElementContent( tempStorage[0]);
        getAndEraseElement(trkseg, "name");
        mergedTrkSegs += trkseg;
    }
    if (! mergedTrkSegs.empty()) {
        source = mergedTrkSegs;
    }

    if (! elementExists(source,"trkpt")) {
        throw domain_error("No 'trkpt' element.");
    } else {
     tempStorage[0] = getAndEraseElement(source, "trkpt");
    }

    if (! attributeExists( tempStorage[0],"lat")) {
        throw domain_error("No 'lat' attribute.");
    } else {
        lat = getElementAttribute( tempStorage[0], "lat");
    }
    if (! attributeExists( tempStorage[0],"lon")) {
        throw domain_error("No 'lon' attribute.");
    } else {
        lon = getElementAttribute( tempStorage[0], "lon");
    }
    tempStorage[0] = getElementContent( tempStorage[0]);
    if (elementExists( tempStorage[0], "ele")) {
         tempStorage[1] = getElement( tempStorage[0], "ele");
        ele = getElementContent( tempStorage[1]);
        Position startPos = Position(lat,lon,ele);
        positions.push_back(startPos);
        reportStr << "Start position added: " << startPos.toString() << endl;

    } else {
        Position startPos = Position(lat,lon);
        positions.push_back(startPos);
        reportStr << "Start position added: " << startPos.toString() << endl;

    }
    if (elementExists( tempStorage[0],"name")) {
         tempStorage[1] = getElement( tempStorage[0],"name");
        tempStorage[2] = getElementContent( tempStorage[1]);
    } else {
        positionNames.push_back(tempStorage[2]);
        arrived.push_back(0);
        departed.push_back(0);
    }

    if (! elementExists( tempStorage[0],"time")) {
        throw domain_error("No 'time' element.");
    } else {
       tempStorage[1] = getElement( tempStorage[0],"time");
       time = getElementContent( tempStorage[1]);
       startTime = currentTime = stringToTime(time);
    }

    Position prevPos = positions.back(), nextPos = positions.back();
    while (elementExists(source, "trkpt")) {
         tempStorage[0] = getAndEraseElement(source, "trkpt");

        if (! attributeExists( tempStorage[0],"lat")) {
            throw domain_error("No 'lat' attribute.");

        } else {
            lat = getElementAttribute( tempStorage[0], "lat");
        }

        if (! attributeExists( tempStorage[0],"lon")) {
            throw domain_error("No 'lon' attribute.");

        } else {
            lon = getElementAttribute( tempStorage[0], "lon");
        }

        tempStorage[0] = getElementContent( tempStorage[0]);

        if (elementExists( tempStorage[0], "ele")) {
             tempStorage[1] = getElement( tempStorage[0], "ele");
            ele = getElementContent( tempStorage[1]);
            nextPos = Position(lat,lon,ele);

        } else {
            nextPos = Position(lat,lon);
        }

        if (! elementExists( tempStorage[0],"time")) {
            throw domain_error("No 'time' element.");
        } else {
            tempStorage[1] = getElement( tempStorage[0],"time");
            time = getElementContent( tempStorage[1]);
            currentTime = stringToTime(time);
        }


        if (areSameLocation(nextPos, prevPos)) {
            // If we're still at the same location, then we haven't departed yet.
            departed.back() = currentTime - startTime;
            reportStr << "Position ignored: " << nextPos.toString() << endl;
        } else {
            if (elementExists( tempStorage[0],"name")) {
                 tempStorage[1] = getElement( tempStorage[0],"name");
                tempStorage[2] = getElementContent( tempStorage[1]);

            } else {
                tempStorage[2] = ""; // Fixed bug by adding this.
                positions.push_back(nextPos);
                positionNames.push_back(tempStorage[2]);
                timeElapsed = currentTime - startTime;
                arrived.push_back(timeElapsed);
                departed.push_back(timeElapsed);
                reportStr << "Position added: " << nextPos.toString() << endl;
                reportStr << " at time: " << to_string(timeElapsed) << endl;
                prevPos = nextPos;
            }
        }
    }
    reportStr << positions.size() << " positions added." << endl;

    Track::calcRouteLength();
    report = reportStr.str();
}

void Track::setGranularity(metres granularity)
{
    bool implemented = false;
    assert(implemented);
}

seconds Track::stringToTime(const std::string & timeStr)
{
    return stoull(timeStr);
}
