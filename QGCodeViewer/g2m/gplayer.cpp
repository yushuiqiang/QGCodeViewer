#include "gplayer.hpp"

namespace g2m 
{

GPlayer::GPlayer()
{
    first = true;
    current_line = 0;
    inv_ds = 1.0 / DEFAULT_STEP_SIZE;
    m = 0;
    play_flag = false;
    feed_rate = DEFAULT_FEED_RATE;
    traverse_feed_rate = DEFAULT_TRAVERSE_FEED_RATE;
    total_length = 0.0;
    total_time = 0.0;
}

GPlayer::~GPlayer() {}

void GPlayer::setStepSize(double ds) 
{
    if (ds > 0.0)
        inv_ds = 1.0 / ds;
}

void GPlayer::setTraverseFeedRate(double rate) 
{
    if (rate > 0.0)
        traverse_feed_rate = rate;
}

canonLine* GPlayer::getCanonLine(unsigned int line) 
{ 
    return lines[line]; 
}

void GPlayer::play() 
{
    if (play_flag == false) 
        {
        play_flag = true;
        qDebug() << " gplayer::play() ";
        emit debugMessage( tr("GPlayer: play") );
        slotRequestMove();
        }
}

void GPlayer::slotRequestMove() 
{
    if (!(lines.size() > 0)) 
        play_flag = false; return; 

    if (current_line >= (lines.size()-1)) 
        play_flag = false; return; 
    
    if (play_flag == false) 
        {
        if (lines.size() - 1)
            emit signalProgress( (int)(100*current_line/(lines.size()-1)) , current_line, total_time, true); // report progress to ui
            
        return;
        }
    // UI request that we signal the next signalToolPosition()
    canonLine* cl = lines[current_line];
    
    if (first) 
        {// first ever call here
        current_tool = cl->getStatus()->getTool();
        first = false;
        }   
    
    if (cl->isMotion() ) 
        {
        // divide motion into sampled points. signal motion along path.
        if (m == 0) 
            {
            move_length = cl->length();
            n_samples = std::max( (int)( move_length * inv_ds ) , 2 ); // want at least two points: start-end
            interval_size = move_length /(double)(n_samples-1);
            motionStatus = cl->getStatus()->getSpindleMotionStatus();
            feed_rate = cl->getStatus()->getFeed();
            if (feed_rate <= 0.0) feed_rate = DEFAULT_FEED_RATE;
            double diff_z = cl->point((n_samples-1)*interval_size).z - cl->point(0.0).z;
            plunge = (diff_z > TOLERANCE) ? POSITIVE_PLUNGE : (diff_z < -TOLERANCE) ? NEGATIVE_PLUNGE : NO_PLUNGE;
            motionStatus |= plunge;
            }
        // FIXME: handle first and last moves differently?
        Point pos = cl->point( (double)(m) * interval_size );
        #ifdef MULTI_AXIS
        Point angle = cl->angle( (double)(m) * interval_size );
        #endif
        if (m == (n_samples-1) )
            move_done = true;
        #ifdef MULTI_AXIS
            emit signalToolPosition( pos.x, pos.y, pos.z, angle.x, angle.y, angle.z, current_line, motionStatus, feed_rate );
        #else
            emit signalToolPosition( pos.x, pos.y, pos.z, current_line, motionStatus, feed_rate);
        #endif
        m++; // advance along the move
        } 
    else 
        {
        // not motion, check for toolchange
        if ( current_tool != cl->getStatus()->getTool() ) 
            {
            emit signalToolChange( cl->getStatus()->getTool() );
            current_tool = cl->getStatus()->getTool();
            }
        current_line++;
        slotRequestMove(); // call myself!
        }
    
    if (move_done) 
        {
        if (cl->isMotion()) 
            {
            total_length += move_length;
            if (cl->getMotionType() == TRAVERSE)
                total_time += move_length / traverse_feed_rate;
            else
                total_time += move_length / feed_rate;
            }
        current_line++;
        move_done = false;
        m = 0;
        }
    if (lines.size() - 1)
        {
        if (m == 0 || (m & DEFAULT_ANIMATE_INTERVAL) == 0x0) 
            emit signalProgress( (int)(100*current_line/(lines.size()-1)) , current_line, total_time, current_line == (lines.size()-1)); // report progress to ui
        }
}

void GPlayer::pause() 
{
    play_flag = false;
    emit debugMessage( tr("GPlayer: pause") );
}


void GPlayer::stop() 
{
    if (play_flag == false)
        emit signalProgress(0, 0, 0.0, true);
    play_flag = false;
    first = true;
    current_line = 0;
    m = 0;
    total_time = 0;
    emit debugMessage( tr("GPlayer: stop") );
}

void GPlayer::appendCanonLine( canonLine* l) 
{
    lines.push_back(l);
}

} // end namespace