"""
Pleaase note that most of the present code is directly taken from the
following article : 
https://medium.com/@guanghuiliang/make-a-simple-python-terminal-loading-animation-by-yourself-422c196fcb3b
"""
from threading import Thread 
from threading import Event
import time
import sys

# have some pretty output
class style:
    RED     = '\033[0;31m'
    BLUE    = '\033[0;34m'
    PURPLE  = '\033[0;35m'
    GREEN   = '\033[0;32m'
    NO_COL  = '\033[0m'
    BOLD    = '\033[1m'
    NORMAL  = '\033[0m'  
    

# parallel loading animation
class TermLoading():
    def __init__(self):
        self.message = ""
        self.finish_message = ""
        self.__failed = False
        self.__finished = False
        self.failed_message = ""
        self.__threadEvent = Event()
        self.__thread = Thread(target=self.__loading, daemon=True)
        self.__threadBlockEvent = Event()

    @property
    def finished(self):
        return self.__finished

    @finished.setter
    def finished(self, finished):
        if isinstance(finished, bool):
            self.__finished = finished
            if finished:
                self.__threadEvent.set()
                time.sleep(0.1)
        else:
            raise ValueError

    @property
    def failed(self):
        return self.__failed

    @failed.setter
    def failed(self, failed):
        if isinstance(failed, bool):
            self.__failed = failed
            if failed:
                self.__threadEvent.set()
                time.sleep(0.1)
        else:
            raise ValueError

    def show(self, loading_message: str, finish_message: str = '✅ Finished', failed_message='❌ Failed'):
        self.message = loading_message
        self.finish_message = finish_message
        self.failed_message = failed_message
        self.show_loading()

    def show_loading(self):
        self.finished = False
        self.failed = False
        self.__threadEvent.clear()
        if not self.__thread.is_alive():
            self.__thread.start()
        else:
            self.__threadBlockEvent.set()

    def __loading(self):
        symbols = ['[    ]', '[.   ]', '[..  ]', '[... ]', '[ ...]', '[  ..]', '[   .]' ]
        i = 0
        while True:
            while not self.finished and not self.failed:
                i = (i + 1) % len(symbols)
                print('\r' + style.BLUE + self.message + " " + symbols[i] + style.NORMAL, flush = True, end='')
                # print('\r\033[K%s %s' % (self.message, symbols[i]), flush=True, end='')
                self.__threadEvent.wait(0.1)
                self.__threadEvent.clear()
            if self.finished is True and not self.failed:
                print('\r' + style.BOLD + style.GREEN + self.finish_message + style.NORMAL, flush=True)
            else:
                print('\r' + style.BOLD + style.RED + self.failed_message + style.NORMAL, flush=True)
            self.__threadBlockEvent.wait()
            self.__threadBlockEvent.clear()

def print_err(message):
    print(style.RED + style.BOLD + "Error : " + message + style.NO_COL, file=sys.stderr)
