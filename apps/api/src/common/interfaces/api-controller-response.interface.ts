export interface ApiControllerResponse<T> {
  message?: string;
  data: T;
}
