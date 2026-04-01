"""Communicate with WiCAN device HTTP-API via available endpoints."""

import logging

import aiohttp

_LOGGER = logging.getLogger(__name__)


class WiCan:
    """WiCan device connection via API endpoints.

    Attributes
    ----------
    ip : Any
        IP-Address or hostname / mDNS name of the WiCAN device.

    """

    ip = ""

    def __init__(self, ip) -> None:
        """Initialize the WiCan API integration with the device IP / name."""
        self.ip = ip

    async def call(self, endpoint, params=None, method="get"):
        """Call WiCan device HTTP-API endpoint and provide response."""
        if params is None:
            params = {}
        match method:
            case "get":
                async with aiohttp.ClientSession() as session:
                    async with session.get(
                        "http://" + self.ip + endpoint, params=params
                    ) as resp:
                        resp.data = await resp.json(content_type=None)
                        return resp

    async def test(self) -> bool:
        """Test if the WiCan device API is reachable and protocol is correct."""
        result = await self.call("/check_status")

        return result.status == 200 and result.data.get("protocol") == "auto_pid"

    async def check_status(self):
        """Check if the WiCan device API is reachable."""
        try:
            result = await self.call("/check_status")
        except:
            return False

        if result.status != 200:
            return False

        return result.data

    async def get_pid(self):
        """Call the WiCan API to receive the car configuration metadata and the current values."""
        try:
            pid_data = await self.call("/autopid_data")
            pid_meta = await self.call("/load_car_config")
        except:
            return False

        if not isinstance(pid_meta.data, dict):
            return False

        # 1. Build a flat dictionary of parameters from the new nested JSON structure
        meta_dict = {}

        # Search inside new "pid_groups"
        if "pid_groups" in pid_meta.data:
            for group in pid_meta.data.get("pid_groups", []):
                for pid in group.get("pids", []):
                    for param in pid.get("parameters", []):
                        name = param.get("name") or param.get("Name")
                        if name:
                            meta_dict[name] = param

        # Search inside "can_filters"
        if "can_filters" in pid_meta.data:
            for filter_obj in pid_meta.data.get("can_filters", []):
                for param in filter_obj.get("parameters", []):
                    name = param.get("name") or param.get("Name")
                    if name:
                        meta_dict[name] = param

        # Search inside legacy "pids" (if present)
        if "pids" in pid_meta.data:
            for pid in pid_meta.data.get("pids", []):
                if "parameters" in pid:
                    for param in pid.get("parameters", []):
                        name = param.get("name") or param.get("Name")
                        if name:
                            meta_dict[name] = param
                else:
                    name = pid.get("name") or pid.get("Name")
                    if name:
                        meta_dict[name] = pid

        # Fallback for very old firmware versions
        if not meta_dict and not any(k in pid_meta.data for k in ["pid_groups", "pids", "can_filters"]):
            meta_dict = pid_meta.data

        # 2. Merge metadata with actual live values
        result = {}
        for key, meta in meta_dict.items():
            result[key] = dict(meta)  # Safe copy to guarantee it is a dictionary
            
            # Safely get the value from pid_data
            value = False
            if hasattr(pid_data, "data") and isinstance(pid_data.data, dict):
                value = pid_data.data.get(key, False)
            
            result[key]["value"] = value

        return result