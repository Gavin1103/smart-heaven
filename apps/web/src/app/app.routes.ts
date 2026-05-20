import { Routes } from '@angular/router';
import { LoginPageComponent } from './features/auth/pages/login-page/login-page';

export const routes: Routes = [
  {
    path: 'login',
    loadComponent: () => LoginPageComponent,
  },
  // {
  //   path: '',
  //   redirectTo: 'login',
  //   pathMatch: 'full',
  // },
];
