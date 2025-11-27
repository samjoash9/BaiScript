import { Routes, Route } from 'react-router-dom';
import IDEPage from './IDEPage';
import Docs from './Docs';

export default function App() {
  return (
    <Routes>
      <Route path="/" element={<IDEPage />} />
      <Route path="/docs" element={<Docs />} />
    </Routes>
  );
}
