<template>
    <BasePage :title="$t('influxadmin.InfluxSettings')" :isLoading="dataLoading">
        <BootstrapAlert v-model="showAlert" dismissible :variant="alertType">
            {{ alertMessage }}
        </BootstrapAlert>

        <form @submit="saveInfluxConfig">
            <CardElement :text="$t('influxadmin.InfluxConfiguration')" textVariant="text-bg-primary">
                <InputElement :label="$t('influxadmin.EnableInflux')"
                              v-model="influxConfigList.influx_enabled"
                              type="checkbox" wide/>
            </CardElement>

            <CardElement :text="$t('influxadmin.InfluxCloudParameter')" textVariant="text-bg-primary" add-space
                         v-show="influxConfigList.influx_enabled"
            >
                <InputElement :label="$t('influxadmin.Hostname')"
                              v-model="influxConfigList.influx_hostname"
                              type="text" maxlength="128"
                              :placeholder="$t('influxadmin.HostnameHint')"/>

                <InputElement :label="$t('influxadmin.Port')"
                              v-model="influxConfigList.influx_port"
                              type="number" min="1" max="65535"/>

                <InputElement :label="$t('influxadmin.Org')"
                              v-model="influxConfigList.influx_org"
                              type="text" maxlength="64"
                              :placeholder="$t('influxadmin.OrgHint')"/>

                <InputElement :label="$t('influxadmin.Token')"
                              v-model="influxConfigList.influx_token"
                              type="password" maxlength="64"
                              :placeholder="$t('influxadmin.TokenHint')"/>

                <InputElement :label="$t('influxadmin.Bucket')"
                              v-model="influxConfigList.influx_bucket"
                              type="text" maxlength="64"
                              :placeholder="$t('influxadmin.BucketHint')"/>

                <InputElement :label="$t('influxadmin.PublishInterval')"
                              v-model="influxConfigList.influx_publish_interval"
                              type="number" min="5" max="86400"
                              :postfix="$t('influxadmin.Seconds')"/>
            </CardElement>
            <button type="submit" class="btn btn-primary mb-3">{{ $t('influxadmin.Save') }}</button>
        </form>
    </BasePage>
</template>

<script lang="ts">
import BasePage from '@/components/BasePage.vue';
import BootstrapAlert from "@/components/BootstrapAlert.vue";
import CardElement from '@/components/CardElement.vue';
import InputElement from '@/components/InputElement.vue';
import type { InfluxConfig } from "@/types/InfluxConfig";
import { authHeader, handleResponse } from '@/utils/authentication';
import { defineComponent } from 'vue';

export default defineComponent({
    components: {
        BasePage,
        BootstrapAlert,
        CardElement,
        InputElement,
    },
    data() {
        return {
            dataLoading: true,
            influxConfigList: {} as InfluxConfig,
            alertMessage: "",
            alertType: "info",
            showAlert: false,
        };
    },
    created() {
        this.getInfluxConfig();
    },
    methods: {
        getInfluxConfig() {
            this.dataLoading = true;
            fetch("/api/influx/config", { headers: authHeader() })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then((data) => {
                    this.influxConfigList = data;
                    this.dataLoading = false;
                });
        },
        saveInfluxConfig(e: Event) {
            e.preventDefault();

            const formData = new FormData();
            formData.append("data", JSON.stringify(this.influxConfigList));

            fetch("/api/influx/config", {
                method: "POST",
                headers: authHeader(),
                body: formData,
            })
                .then((response) => handleResponse(response, this.$emitter, this.$router))
                .then(
                    (response) => {
                        this.alertMessage = this.$t('apiresponse.' + response.code, response.param);
                        this.alertType = response.type;
                        this.showAlert = true;
                    }
                );
        },
    },
});
</script>
