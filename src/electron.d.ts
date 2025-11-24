interface ElectronAPI {
  runCompiler: (sourceCode: string) => Promise<{
    success: boolean;
    exitCode?: number;
    stdout?: string;
    stderr?: string;
    error?: string;
    outputs?: {
      print?: string;
      assembly?: string;
      machine?: string;
    };
  }>;
  windowMinimize: () => Promise<void>;
  windowMaximize: () => Promise<void>;
  windowClose: () => Promise<void>;
  windowIsMaximized: () => Promise<boolean>;
}

declare global {
  interface Window {
    electronAPI?: ElectronAPI;
  }
}

export {};

