// Copyright (c) 2012 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "custom_scheme.h"

#include <algorithm>
#include <string>
#include <fstream>

#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_request.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"

namespace client {
namespace scheme_test {

namespace {
    
// Implementation of the schema handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler {
 public:
  ClientSchemeHandler() : offset_(0) {}

  virtual bool ProcessRequest(CefRefPtr<CefRequest> request,
                              CefRefPtr<CefCallback> callback)
                              OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool handled = false;	
    std::string url = request->GetURL();

	int scheme_and_domain_length = 15; // Size of: "client://nkcef/"
	std::size_t found = url.find('?');
    std::string fileName;

    if (found != std::string::npos)
    {
        fileName = url.substr(scheme_and_domain_length, found - scheme_and_domain_length);
    }
    else
    {
        fileName = url.substr(scheme_and_domain_length, url.length() - scheme_and_domain_length);
    }

                                  fileName = "nkcef.app/Contents/Resources/" + fileName;
  std::string content = getFileContent(fileName.c_str());
  data_ = content;
  
                                  if (data_.empty()) {
                                      data_ = "Not found!<br />File: " + fileName;
                                      LOG(INFO) << "File not found: " << fileName;
                                  }

    handled = true;
                                  
                                  if (fileName.substr(fileName.find_last_of(".") + 1) == "png") {
                                      mime_type_ = "image/png";
                                  } else if (fileName.substr(fileName.find_last_of(".") + 1) == "jpg") {
                                      mime_type_ = "image/jpg";
                                  } else if (fileName.substr(fileName.find_last_of(".") + 1) == "ico") {
                                      mime_type_ = "image/ico";
                                  } else if (fileName.substr(fileName.find_last_of(".") + 1) == "gif") {
                                      mime_type_ = "image/gif";
                                  } else if (fileName.substr(fileName.find_last_of(".") + 1) == "css") {
                                      mime_type_ = "text/css";
                                  } else {
                                      mime_type_ = "text/html";
                                  }
                                  
                                  
    if (handled) {
      // Indicate the headers are available.
      callback->Continue();
      return true;
    }

    return false;
  }
    
    std::string
    getFileContent(const std::string& path)
    {
        std::ifstream file(path);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        return content;
    }

  virtual void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                  int64& response_length,
                                  CefString& redirectUrl) OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    DCHECK(!data_.empty());

    response->SetMimeType(mime_type_);
    response->SetStatus(200);

    // Set the resulting response length
    response_length = data_.length();
  }

  virtual void Cancel() OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
  }

  virtual bool ReadResponse(void* data_out,
                            int bytes_to_read,
                            int& bytes_read,
                            CefRefPtr<CefCallback> callback)
                            OVERRIDE {
    CEF_REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    if (offset_ < data_.length()) {
      // Copy the next block of data into the buffer.
      int transfer_size =
          std::min(bytes_to_read, static_cast<int>(data_.length() - offset_));
      memcpy(data_out, data_.c_str() + offset_, transfer_size);
      offset_ += transfer_size;

      bytes_read = transfer_size;
      has_data = true;
    }

    return has_data;
  }

 private:
  std::string data_;
  std::string mime_type_;
  size_t offset_;

  IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
};

// Implementation of the factory for for creating schema handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
 public:
  // Return a new scheme handler instance to handle the request.
  virtual CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                               CefRefPtr<CefFrame> frame,
                                               const CefString& scheme_name,
                                               CefRefPtr<CefRequest> request)
                                               OVERRIDE {
    CEF_REQUIRE_IO_THREAD();
    return new ClientSchemeHandler();
  }

  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
};

}  // namespace

void RegisterSchemeHandlers() {
  CefRegisterSchemeHandlerFactory("client", "nkcef",
      new ClientSchemeHandlerFactory());
}

}  // namespace scheme_test
}  // namespace client
