#ifndef STEPPERSPIWATCHER_H
#define STEPPERSPIWATCHER_H

#include <QObject>
#include "l6470/l6470.h"
#include <unistd.h>
#include "btypes.h"

class stepperSpiWatcher : public QObject
{
    Q_OBJECT

public:
    explicit stepperSpiWatcher(l6470Status *ms, QObject *parent = 0);
    ~stepperSpiWatcher();

private:
    l6470Status *motorStatus;
    bool cancel;

signals:
    void limitReached(void);
    void updateMotorStatus(void);

public slots:
    void _onWatchLimitsRequest(int low, int high);
};

#endif // STEPPERSPIWATCHER_H
