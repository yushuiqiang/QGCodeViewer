/*
 *  Modification & Copyright 2015      Kazuyasu Hamada (k-hamada@gifu-u.ac.jp)
*/

#ifndef GPLAYER_HH
#define GPLAYER_HH

#include <vector>
#include <limits.h>
#include <iostream>
#include <fstream>

#include <QString>
#include <QProcess>
#include <QObject>
#include <QtDebug>

#include "canonLine.hpp"
#include "nanotimer.hpp"

#define DEFAULT_STEP_SIZE (0.1)
#define DEFAULT_FEED_RATE       (200.0)
#define DEFAULT_TRAVERSE_FEED_RATE      (1000.0)
#define TOLERANCE       (1e-2)
#define DEFAULT_ANIMATE_INTERVAL        (3)

namespace g2m 
{

enum PLUNGE_STATUS { NO_PLUNGE = 0x0, POSITIVE_PLUNGE = 0x10000, NEGATIVE_PLUNGE = 0x20000, };

/**
\class GPlayer
\brief This class loops through the list of canonLine objects produced by g2m.
*/
class GPlayer : public QObject 
{
    Q_OBJECT;

    public:
        GPlayer();
        virtual ~GPlayer();

        void setStepSize(double ds);

        void setTraverseFeedRate(double rate);

        canonLine* getCanonLine(unsigned int line);

    public slots:
        
        /// start or resume executing the program
        virtual void play();
        
        /// signal the next move
        virtual void slotRequestMove() ;
        
        /// pause program
        void pause();
        
        /// stop the execution of the g-code program
        void stop();
        
        /// add a canonLine to the vector of lines to be processed
        /// \param l the new canonLine to be added
        void appendCanonLine( canonLine* l);

    signals:
        /// signal a new tool position
#ifdef MULTI_AXIS
    	void signalToolPosition( double x, double y, double z, double a, double b, double c, int line, int mstatus, double feedrate ); // 5-axis for now..
#else
        void signalToolPosition( double x, double y, double z, int line, int mstatus, double feedrate ); // 3-axis.
#endif
        /// signal a tool change to new tool \param t
        void signalToolChange( int t );
        /// signal the UI how far along the g-code program we are
        void signalProgress( int p, int line, double time, bool force );
        /// signal a debug message
        void debugMessage(QString s);

    protected:
        /// flag for first move of g-code
        bool first;
        /// index of current tool
        int current_tool;
        /// the current canonLine being processed
        unsigned int current_line;
        /// loop variable
        int    m;
        double move_length;
        int    n_samples;
        double interval_size;
        /// flag indicating when current move done
        bool move_done;
        /// vector of canonLines to process
        std::vector<canonLine*> lines;

        bool play_flag;

    private:
        int    plunge;
        int    motionStatus;
        double inv_ds;
		double feed_rate;
		double traverse_feed_rate;
        double total_length;
		double total_time;
};

} // end namespace
#endif // GPlayer_HH
