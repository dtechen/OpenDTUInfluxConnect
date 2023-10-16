<template>
    <BasePage :title="$t('influxinfo.InfluxInformation')" :isLoading="dataLoading" :show-reload="true" @reload="getInfluxInfo">
        <CardElement :text="$t('influxinfo.ConfigurationSummary')" textVariant="text-bg-primary">
            <div class="table-responsive">
                <table class="table table-hover table-condensed">
                    <tbody>
                        <tr>
                            <th>{{ $t('influxinfo.Status') }}</th>
                            <td>
                                <StatusBadge :status="influxDataList.influx_enabled" true_text="influxinfo.Enabled" false_text="influxinfo.Disabled" />
                            </td>
                        </tr>
                        <tr>
                            <th>{{ $t('influxinfo.Server') }}</th>
                            <td>{{ influxDataList.influx_hostname }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('influxinfo.Org') }}</th>
                            <td>{{ influxDataList.influx_org }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('influxinfo.Bucket') }}</th>
                            <td>{{ influxDataList.influx_bucket }}</td>
                        </tr>
                        <tr>
                            <th>{{ $t('influxinfo.PublishInterval') }}</th>
                            <td>{{ $t('influxinfo.Seconds', { sec: influxDataList.influx_publish_interval }) }}</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </CardElement>
    </BasePage>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import CardElement from '@/components/CardElement.vue';
import StatusBadge from '@/components/StatusBadge.vue';
import type { InfluxStatus } from '@/types/InfluxStatus';
import { authHeader, handleResponse } from '@/utils/authentication';
import { defineComponent } from 'vue';

export default defineComponent({
    components: {
        BasePage,
        CardElement,
        StatusBadge
    },
    data() {
        return {
            dataLoading: true,
            influxDataList: {} as InfluxStatus,
        };
    },
    created() {
        this.getInfluxInfo();
    },
    methods: {
        getInfluxInfo() {
            this.dataLoading = true;
            fetch("/api/influx/status", { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.influxDataList = data;
                    this.dataLoading = false;
                });
        },
    },
});
</script>
