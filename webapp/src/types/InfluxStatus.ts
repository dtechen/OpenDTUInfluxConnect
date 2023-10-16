export interface InfluxStatus {
    influx_enabled: boolean;
    influx_hostname: string;
    influx_org: string;
    influx_token: string;
    influx_bucket: string;
    influx_publish_interval: string;
}