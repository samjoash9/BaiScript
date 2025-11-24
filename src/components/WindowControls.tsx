import { useState, useEffect } from 'react';

export function WindowControls() {
  const [isMaximized, setIsMaximized] = useState(false);

  useEffect(() => {
    // Check if window is maximized on mount and when it changes
    if (window.electronAPI) {
      window.electronAPI.windowIsMaximized().then(setIsMaximized);

      // Listen for maximize/unmaximize events
      const checkMaximized = async () => {
        const maximized = await window.electronAPI?.windowIsMaximized();
        if (maximized !== undefined) {
          setIsMaximized(maximized);
        }
      };

      // Check periodically (Electron doesn't have a direct event for this)
      const interval = setInterval(checkMaximized, 100);
      return () => clearInterval(interval);
    }
  }, []);

  const handleMinimize = () => {
    if (window.electronAPI) {
      window.electronAPI.windowMinimize();
    }
  };

  const handleMaximize = () => {
    if (window.electronAPI) {
      window.electronAPI.windowMaximize();
      setIsMaximized(prev => !prev);
    }
  };

  const handleClose = () => {
    if (window.electronAPI) {
      window.electronAPI.windowClose();
    }
  };

  return (
    <div style={{
      display: 'flex',
      alignItems: 'center',
      gap: '10px',
      marginLeft: '8px',
      zIndex: 1000,
      position: 'relative'
    }}>
      {/* Yellow (Minimize) */}
      <button
        onClick={handleMinimize}
        title="Minimize"
        disabled={!window.electronAPI}
        onMouseEnter={(e) => {
          if (window.electronAPI) {
            e.currentTarget.style.transform = 'scale(1.1)';
            e.currentTarget.style.backgroundColor = '#ff9500';
          }
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.transform = 'scale(1)';
          e.currentTarget.style.backgroundColor = '#ffbd2e';
        }}
        style={{
          width: '16px',
          height: '16px',
          minWidth: '16px',
          minHeight: '16px',
          borderRadius: '50%',
          backgroundColor: '#ffbd2e',
          border: 'none',
          cursor: window.electronAPI ? 'pointer' : 'default',
          opacity: window.electronAPI ? 1 : 1,
          flexShrink: 0,
          padding: '0',
          margin: '0',
          transition: 'all 0.2s ease',
          boxShadow: '0 2px 4px rgba(0,0,0,0.4)',
          outline: 'none',
          position: 'relative',
          zIndex: 1001
        }}
      />

      {/* Green (Maximize) */}
      <button
        onClick={handleMaximize}
        title={isMaximized ? "Restore" : "Maximize"}
        disabled={!window.electronAPI}
        onMouseEnter={(e) => {
          if (window.electronAPI) {
            e.currentTarget.style.transform = 'scale(1.1)';
            e.currentTarget.style.backgroundColor = '#20d148';
          }
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.transform = 'scale(1)';
          e.currentTarget.style.backgroundColor = '#28c840';
        }}
        style={{
          width: '16px',
          height: '16px',
          minWidth: '16px',
          minHeight: '16px',
          borderRadius: '50%',
          backgroundColor: '#28c840',
          border: 'none',
          cursor: window.electronAPI ? 'pointer' : 'default',
          opacity: window.electronAPI ? 1 : 1,
          flexShrink: 0,
          display: 'block',
          padding: '0',
          margin: '0',
          transition: 'all 0.2s ease',
          boxShadow: '0 2px 4px rgba(0,0,0,0.4)',
          outline: 'none',
          position: 'relative',
          zIndex: 1001
        }}
      />

      {/* Red (Close) */}
      <button
        onClick={handleClose}
        title="Close"
        disabled={!window.electronAPI}
        onMouseEnter={(e) => {
          if (window.electronAPI) {
            e.currentTarget.style.transform = 'scale(1.1)';
            e.currentTarget.style.backgroundColor = '#ff3b30';
          }
        }}
        onMouseLeave={(e) => {
          e.currentTarget.style.transform = 'scale(1)';
          e.currentTarget.style.backgroundColor = '#ff5f57';
        }}
        style={{
          width: '16px',
          height: '16px',
          minWidth: '16px',
          minHeight: '16px',
          borderRadius: '50%',
          backgroundColor: '#ff5f57',
          border: 'none',
          cursor: window.electronAPI ? 'pointer' : 'default',
          opacity: window.electronAPI ? 1 : 1,
          flexShrink: 0,
          display: 'block',
          padding: '0',
          margin: '0',
          transition: 'all 0.2s ease',
          boxShadow: '0 2px 4px rgba(0,0,0,0.4)',
          outline: 'none',
          position: 'relative',
          zIndex: 1001
        }}
      />
    </div>
  );
}

