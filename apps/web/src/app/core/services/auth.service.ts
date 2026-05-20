import { Injectable, inject } from '@angular/core';
import { Observable, tap } from 'rxjs';
import { ApiService } from './api.service';
import { ApiResponse, JwtResponse, LoginRequest } from '@sm/shared-types';

@Injectable({
  providedIn: 'root',
})
export class AuthService {
  private api = inject(ApiService);

  login(payload: LoginRequest): Observable<ApiResponse<JwtResponse>> {
    return this.api.post<ApiResponse<JwtResponse>, LoginRequest>('/auth/login', payload).pipe(
      tap((response) => {
        localStorage.setItem('token', response.data.token);
      }),
    );
  }

  logout(): void {
    localStorage.removeItem('token');
  }

  getToken(): string | null {
    return localStorage.getItem('token');
  }

  isLoggedIn(): boolean {
    return !!this.getToken();
  }
}
