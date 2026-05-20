import { Injectable, inject } from '@angular/core';
import { HttpClient, HttpHeaders, HttpParams } from '@angular/common/http';
import { Observable } from 'rxjs';
import { API_BASE_URL } from '../constants/api.constants';

/**
 * ApiService
 *
 * A centralized service for handling HTTP requests to the backend API.
 *
 * This service provides generic methods for common HTTP operations:
 * - GET
 * - POST
 * - PUT
 * - PATCH
 * - DELETE
 *
 * It automatically prefixes all endpoints with the base API URL
 * and allows optional query parameters and custom headers.
 *
 * This service should be used as a low-level abstraction.
 * Feature-specific services (e.g., AuthService, PatientsService)
 * should build on top of this service.
 *
 *  * NOTE:
 *  * This service does NOT handle authentication headers.
 *  * Authentication is handled globally via an HTTP interceptor.
 */
@Injectable({
  providedIn: 'root',
})
export class ApiService {
  private http = inject(HttpClient);
  private readonly baseUrl = API_BASE_URL;

  /**
   * Sends a GET request.
   *
   * @param endpoint - API endpoint (e.g., '/patients')
   * @param options - Optional query parameters and headers
   * @returns Observable of type T
   */
  get<T>(
    endpoint: string,
    options?: {
      params?: Record<string, string | number | boolean>;
      headers?: Record<string, string>;
    },
  ): Observable<T> {
    return this.http.get<T>(`${this.baseUrl}${endpoint}`, {
      params: this.createParams(options?.params),
      headers: this.createHeaders(options?.headers),
    });
  }

  /**
   * Sends a POST request.
   *
   * @param endpoint - API endpoint (e.g., '/auth/login')
   * @param body - Request payload
   * @param options - Optional query parameters and headers
   * @returns Observable of type TResponse
   */
  post<TResponse, TBody>(
    endpoint: string,
    body: TBody,
    options?: {
      params?: Record<string, string | number | boolean>;
      headers?: Record<string, string>;
    },
  ): Observable<TResponse> {
    return this.http.post<TResponse>(`${this.baseUrl}${endpoint}`, body, {
      params: this.createParams(options?.params),
      headers: this.createHeaders(options?.headers),
    });
  }

  /**
   * Sends a PUT request.
   *
   * @param endpoint - API endpoint (e.g., '/users/1')
   * @param body - Request payload
   * @param options - Optional query parameters and headers
   * @returns Observable of type TResponse
   */
  put<TResponse, TBody>(
    endpoint: string,
    body: TBody,
    options?: {
      params?: Record<string, string | number | boolean>;
      headers?: Record<string, string>;
    },
  ): Observable<TResponse> {
    return this.http.put<TResponse>(`${this.baseUrl}${endpoint}`, body, {
      params: this.createParams(options?.params),
      headers: this.createHeaders(options?.headers),
    });
  }

  /**
   * Sends a PATCH request.
   *
   * @param endpoint - API endpoint (e.g., '/users/1')
   * @param body - Partial update payload
   * @param options - Optional query parameters and headers
   * @returns Observable of type TResponse
   */
  patch<TResponse, TBody>(
    endpoint: string,
    body: TBody,
    options?: {
      params?: Record<string, string | number | boolean>;
      headers?: Record<string, string>;
    },
  ): Observable<TResponse> {
    return this.http.patch<TResponse>(`${this.baseUrl}${endpoint}`, body, {
      params: this.createParams(options?.params),
      headers: this.createHeaders(options?.headers),
    });
  }

  /**
   * Sends a DELETE request.
   *
   * @param endpoint - API endpoint (e.g., '/users/1')
   * @param options - Optional query parameters and headers
   * @returns Observable of type T
   */
  delete<T>(
    endpoint: string,
    options?: {
      params?: Record<string, string | number | boolean>;
      headers?: Record<string, string>;
    },
  ): Observable<T> {
    return this.http.delete<T>(`${this.baseUrl}${endpoint}`, {
      params: this.createParams(options?.params),
      headers: this.createHeaders(options?.headers),
    });
  }

  /**
   * Converts a key-value object into HttpParams.
   *
   * @param params - Object containing query parameters
   * @returns HttpParams or undefined if no params provided
   */
  private createParams(params?: Record<string, string | number | boolean>): HttpParams | undefined {
    if (!params) {
      return undefined;
    }

    let httpParams = new HttpParams();

    for (const [key, value] of Object.entries(params)) {
      httpParams = httpParams.set(key, String(value));
    }

    return httpParams;
  }

  /**
   * Converts a key-value object into HttpHeaders.
   *
   * @param headers - Object containing header values
   * @returns HttpHeaders or undefined if no headers provided
   */
  private createHeaders(headers?: Record<string, string>): HttpHeaders | undefined {
    if (!headers) {
      return undefined;
    }

    return new HttpHeaders(headers);
  }
}
