#ifndef W_PAGE_H
#define W_PAGE_H

#include <Arduino.h>
#include "WStringStream.h"


class WPage {
public:

    WPage* next;
    WPage(const char* id, const char* title) {
        this->next = nullptr;
        this->id = id;
        this->title = title;
        this->onPrintPage = nullptr;
        this->onSubmittedPage = nullptr;
	}

	~WPage() {
		delete this->id;
		delete this->title;
	}
    typedef std::function<void(AsyncWebServerRequest*, AsyncResponseStream*)> TCommandPage;

    virtual void printPage(AsyncWebServerRequest *request, AsyncResponseStream* page) {
        if (onPrintPage) onPrintPage(request, page);
    }

    virtual void submittedPage(AsyncWebServerRequest *request, AsyncResponseStream* page) {
        if (onSubmittedPage) onSubmittedPage(request, page);
    }

    void setPrintPage(TCommandPage onPrintPage) {
        this->onPrintPage = onPrintPage;
    }

    void setSubmittedPage(TCommandPage onSubmittedPage) {
        this->onSubmittedPage = onSubmittedPage;
    }

	const char* getId() {
		return id;
	}

	const char* getTitle() {
		return title;
	}


protected:

private:
	const char* id;
	const char* title;
    TCommandPage onPrintPage;
    TCommandPage onSubmittedPage;

};

#endif
