// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "muon_app.h"

#include <windows.h>

// Entry point function for all processes.
int APIENTRY wWinMain(HINSTANCE hInstance, void* sandbox_info) {
  // Provide CEF with command-line arguments.
  CefMainArgs main_args(hInstance);

  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  CefRefPtr<MuonApp> app(new MuonApp);
  int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Parse command-line arguments for use in this method.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(GetCommandLineW());

  // Specify CEF global settings here.
  CefSettings settings;

  // When generating projects with CMake the CEF_USE_SANDBOX value will be
  // defined automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line
  // to disable use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // MuonApp implements application-level callbacks for the browser process.
  // It will create the first browser instance in OnContextInitialized() after
  // CEF has initialized.

  // Initialize the CEF browser process. May return false if initialization
  // fails or if early exit is desired (for example, due to process singleton
  // relaunch behavior).
  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    return 1;
  }

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}

